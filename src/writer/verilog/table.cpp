#include "writer/verilog/table.h"

#include "iroha/i_design.h"
#include "iroha/logging.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"
#include "iroha/stl_util.h"
#include "writer/module_template.h"
#include "writer/verilog/embed.h"
#include "writer/verilog/insn_writer.h"
#include "writer/verilog/internal_sram.h"
#include "writer/verilog/module.h"
#include "writer/verilog/ports.h"
#include "writer/verilog/resource.h"
#include "writer/verilog/state.h"
#include "writer/verilog/task.h"

namespace iroha {
namespace writer {
namespace verilog {

Table::Table(ITable *table, Ports *ports, Module *mod, Embed *embed,
	     ModuleTemplate *tmpl)
  : i_table_(table), ports_(ports), mod_(mod), embed_(embed),
    tmpl_(tmpl) {
  table_id_ = table->GetId();
  st_ = "st_" + Util::Itoa(table_id_);
  task_.reset(Task::MayCreateTask(this));
}

Table::~Table() {
  STLDeleteValues(&states_);
}

void Table::Build() {
  for (auto *i_state : i_table_->states_) {
    State *st = new State(i_state, this);
    st->Build();
    states_.push_back(st);
  }

  BuildStateDecl();
  BuildRegister();
  BuildResource();
  BuildInsnOutputWire();
  BuildSharedRegisters();
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
  sd << "\n";

  ostream &sv = tmpl_->GetStream(kStateDeclSection);
  sv << "  reg [" << bits << ":0] " << StateVariable() << ";\n";
}

void Table::BuildResource() {
  for (auto *res : i_table_->resources_) {
    unique_ptr<Resource> builder(Resource::Create(*res, *this));
    if (builder.get()) {
      builder->Build();
    }
  }
}

void Table::BuildRegister() {
  ostream &rs = tmpl_->GetStream(kRegisterSection);
  ostream &is = tmpl_->GetStream(kInitialValueSection + Util::Itoa(table_id_));
  for (auto *reg : i_table_->registers_) {
    if (!reg->IsConst()) {
      if (reg->IsStateLocal()) {
	rs << "  wire";
      } else {
	rs << "  reg";
      }
      rs << " " << WidthSpec(reg);
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
	rs << "  wire " << WidthSpec(oreg) << " "
	   << InsnWriter::InsnOutputWireName(*insn, nth)
	   << ";\n";
      }
    }
  }
}

void Table::BuildSharedRegisters() {
  const IModule *i_mod = mod_->GetIModule();
  map<IRegister *, vector<ITable *>> writers;
  // Collects tables which writes to this register.
  for (auto *tab : i_mod->tables_) {
    if (tab->GetId() == i_table_->GetId()) {
      continue;
    }
    for (auto *res : tab->resources_) {
      IRegister *reg = res->GetForeignRegister();
      if (reg == nullptr || reg->GetTable() != i_table_) {
	continue;
      }
      writers[reg].push_back(tab);
    }
  }

  ostream &os = tmpl_->GetStream(kStateOutput + Util::Itoa(table_id_));
  for (auto &w : writers) {
    IRegister *reg = w.first;
    bool is_first = true;
    for (auto *t : w.second) {
      os << "      ";
      if (!is_first) {
	os << "else ";
      }
      string s = SharedRegPrefix(*t, *reg);
      os << "if (" << s << "_w) begin\n";
      os << "        " << InsnWriter::RegisterName(*reg) << " <= ";
      os << s << "_wdata;\n";
      os << "      end\n";
      is_first = false;
    }
  }
}

string Table::WidthSpec(const IRegister *reg) {
  if (reg->value_type_.GetWidth() > 0) {
    return " [" + Util::Itoa(reg->value_type_.GetWidth() - 1) + ":0]";
  }
  return string();
}

string Table::SharedRegPrefix(const ITable &writer, const IRegister &reg) const {
  return "shared_reg_" + Util::Itoa(writer.GetId()) + "_" + Util::Itoa(reg.GetTable()->GetId()) + "_" + Util::Itoa(reg.GetId());
}

void Table::Write(ostream &os) {
  os << "  always @(posedge " << ports_->GetClk() << ") begin\n";
  os << "    if (";
  if (!mod_->GetResetPolarity()) {
    os << "!";
  }
  os << ports_->GetReset() << ") begin\n";
  if (!IsEmpty()) {
    os << "      " << StateVariable() << " <= `";
    if (IsTask()) {
      os << StateName(Task::kTaskEntryStateId);
    } else {
      os << InitialStateName();
    }
    os << ";\n";
  }
  os << tmpl_->GetContents(kInitialValueSection + Util::Itoa(table_id_));
  os << "    end else begin\n";
  os << tmpl_->GetContents(kStateOutput + Util::Itoa(table_id_));
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
  os << "    end\n";
  os << "  end\n";
}

ITable *Table::GetITable() const {
  return i_table_;
}

const string &Table::StateVariable() const {
  return st_;
}

string Table::StateName(int id) {
  string n = "S_" + Util::Itoa(table_id_) + "_";
  if (id == Task::kTaskEntryStateId) {
    return n + "task_idle";
  } else {
    return n + Util::Itoa(id);
  }
}

ModuleTemplate *Table::GetModuleTemplate() const {
  return tmpl_;
}

string Table::InitialStateName() {
  IState *initial_st = i_table_->GetInitialState();
  if (initial_st == nullptr) {
    LOG(FATAL) << "null initial state.\n";
  }
  return StateName(initial_st->GetId());
}

bool Table::IsTask() {
  return (task_.get() != nullptr);
}

bool Table::IsEmpty() {
  return (states_.size() == 0);
}

Ports *Table::GetPorts() const {
  return ports_;
}

Embed *Table::GetEmbed() const {
  return embed_;
}

Module *Table::GetModule() const {
  return mod_;
}

Task *Table::GetTask() const {
  return task_.get();
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
