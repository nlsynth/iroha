#include "writer/verilog/table.h"

#include "design/design_util.h"
#include "iroha/i_design.h"
#include "iroha/logging.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"
#include "iroha/stl_util.h"
#include "writer/module_template.h"
#include "writer/verilog/embed.h"
#include "writer/verilog/insn_writer.h"
#include "writer/verilog/module.h"
#include "writer/verilog/ports.h"
#include "writer/verilog/resource.h"
#include "writer/verilog/shared_reg.h"
#include "writer/verilog/state.h"
#include "writer/verilog/task.h"

namespace iroha {
namespace writer {
namespace verilog {

Table::Table(ITable *table, Ports *ports, Module *mod, EmbeddedModules *embed,
	     ModuleTemplate *tmpl)
  : i_table_(table), ports_(ports), mod_(mod), embedded_modules_(embed),
    tmpl_(tmpl) {
  table_id_ = table->GetId();
  st_ = "st_" + Util::Itoa(table->GetId());
  is_task_ = Task::IsTask(*this);
}

Table::~Table() {
  STLDeleteValues(&states_);
}

void Table::Build() {
  BuildStates();

  BuildStateDecl();
  BuildRegister();
  BuildResource();
  BuildInsnOutputWire();
  BuildMultiCycleStateReg();
  SharedReg::BuildSharedRegisters(*this);
}

void Table::BuildStates() {
  for (auto *i_state : i_table_->states_) {
    State *st = new State(i_state, this);
    st->Build();
    states_.push_back(st);
  }
}

void Table::BuildStateDecl() {
  if (states_.size() == 0) {
    return;
  }
  ostream &sd = tmpl_->GetStream(kStateDeclSection);

  int max_id = 0;
  for (auto *st : i_table_->states_) {
    int id = st->GetId();
    sd << "  `define " << StateName(id) << " "
       << id << "\n";
    if (id > max_id) {
      max_id = id;
    }
  }
  if (IsTask()) {
    ++max_id;
    sd << "  `define " << StateName(Task::kTaskEntryStateId) << " "
       << max_id << "\n";
  }
  int bits = 0;
  int u = 1;
  while (u < max_id) {
    u *= 2;
    ++bits;
  }
  sd << "  reg [" << bits << ":0] " << StateVariable() << ";\n";
  sd << "\n";
}

void Table::BuildResource() {
  for (auto *res : i_table_->resources_) {
    unique_ptr<Resource> builder(Resource::Create(*res, *this));
    if (builder.get()) {
      builder->BuildResource();
    }
  }
}

void Table::BuildRegister() {
  ostream &rs = tmpl_->GetStream(kRegisterSection);
  ostream &is = InitialValueSectionStream();
  for (auto *reg : i_table_->registers_) {
    if (!reg->IsConst()) {
      if (reg->IsStateLocal()) {
	rs << "  wire";
      } else {
	rs << "  reg";
      }
      rs << " " << WidthSpec(reg->value_type_);
      rs << " " << reg->GetName() << ";\n";
    }
    if (!reg->IsConst() && reg->HasInitialValue()) {
      is << "      " << reg->GetName() << " <= "
	 << reg->GetInitialValue().value_ << ";\n";
    }
  }
}

void Table::BuildInsnOutputWire() {
  ostream &rs = tmpl_->GetStream(kInsnWireDeclSection);
  for (IState *st : i_table_->states_) {
    for (IInsn *insn : st->insns_) {
      int nth = 0;
      for (IRegister *oreg : insn->outputs_) {
	rs << "  wire " << WidthSpec(oreg->value_type_) << " "
	   << InsnWriter::InsnOutputWireName(*insn, nth)
	   << ";\n";
      }
    }
  }
}

void Table::BuildMultiCycleStateReg() {
  set<IResource *> mc_resources;
  for (IState *st : i_table_->states_) {
    for (IInsn *insn : st->insns_) {
      if (DesignUtil::IsMultiCycleInsn(insn)) {
	mc_resources.insert(insn->GetResource());
      }
    }
  }
  ostream &ws = tmpl_->GetStream(kInsnWireDeclSection);
  for (IResource *res : mc_resources) {
    string w = InsnWriter::MultiCycleStateName(*res);
    ws << "  reg [1:0] " << w << ";\n";
    ostream &is = InitialValueSectionStream();
    is << "      " << w << " <= 0;\n";
  }
}

string Table::WidthSpec(const IValueType &type) {
  if (type.GetWidth() > 0) {
    string s = " [" + Util::Itoa(type.GetWidth() - 1) + ":0]";
    if (type.IsSigned()) {
      s = " signed" + s;
    }
    return s;
  }
  return string();
}

void Table::Write(ostream &os) {
  os << "  // Table " << table_id_ << "\n"
     << "  always @(posedge " << ports_->GetClk() << ") begin\n"
     << "    if (";
  if (!mod_->GetResetPolarity()) {
    os << "!";
  }
  os << ports_->GetReset() << ") begin\n";
  WriteReset(os);
  os << "    end else begin\n";
  WriteBody(os);
  os << "    end\n";
  os << "  end\n";
}

void Table::WriteReset(ostream &os) {
  if (!IsEmpty()) {
    os << "      " << StateVariable() << " <= `";
    if (IsTask()) {
      os << StateName(Task::kTaskEntryStateId);
    } else {
      os << InitialStateName();
    }
    os << ";\n";
  }
  os << InitialValueSectionContents();
}

void Table::WriteBody(ostream &os) {
  os << StateOutputSectionContents();
  if (!IsEmpty()) {
    os << "      case (" << StateVariable() << ")\n";
    if (IsTask()) {
      State::WriteTaskEntry(this, os);
    }
    for (auto *state : states_) {
      state->Write(os);
    }
    os << "      endcase\n";
  }
}

ITable *Table::GetITable() const {
  return i_table_;
}

const string &Table::StateVariable() const {
  return st_;
}

string Table::StateName(int id) const {
  return StateNameFromTable(*i_table_, id);
}

string Table::StateNameFromTable(const ITable &tab, int id) {
  string n = "S_" + Util::Itoa(tab.GetId()) + "_";
  if (id == Task::kTaskEntryStateId) {
    return n + "task_idle";
  } else {
    return n + Util::Itoa(id);
  }
}

ModuleTemplate *Table::GetModuleTemplate() const {
  return tmpl_;
}

ostream &Table::StateOutputSectionStream() const {
  return tmpl_->GetStream(kStateOutputSection + Util::Itoa(table_id_));
}

string Table::StateOutputSectionContents() const {
  return tmpl_->GetContents(kStateOutputSection + Util::Itoa(table_id_));
}

ostream &Table::InitialValueSectionStream() const {
  return tmpl_->GetStream(kInitialValueSection + Util::Itoa(table_id_));
}

string Table::InitialValueSectionContents() const {
  return tmpl_->GetContents(kInitialValueSection + Util::Itoa(table_id_));
}

ostream &Table::TaskEntrySectionStream() const {
  return tmpl_->GetStream(kTaskEntrySection + Util::Itoa(table_id_));
}

string Table::TaskEntrySectionContents() const {
  return tmpl_->GetContents(kTaskEntrySection + Util::Itoa(table_id_));
}

string Table::InitialStateName() {
  IState *initial_st = i_table_->GetInitialState();
  if (initial_st == nullptr) {
    LOG(FATAL) << "null initial state.\n";
  }
  return StateName(initial_st->GetId());
}

bool Table::IsTask() {
  return is_task_;
}

bool Table::IsEmpty() {
  return (states_.size() == 0);
}

Ports *Table::GetPorts() const {
  return ports_;
}

EmbeddedModules *Table::GetEmbeddedModules() const {
  return embedded_modules_;
}

Module *Table::GetModule() const {
  return mod_;
}

Task *Table::GetTask() const {
  return nullptr;
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
