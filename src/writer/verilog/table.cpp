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
    auto *klass = res->GetClass();
    auto *params = res->GetParams();
    if (klass->GetName() == resource::kExtInput) {
      string input_port;
      int width;
      params->GetExtInputPort(&input_port, &width);
      ports_->AddPort(input_port, Port::INPUT, width);
    }
    if (klass->GetName() == resource::kExtOutput) {
      string output_port;
      int width;
      params->GetExtOutputPort(&output_port, &width);
      ports_->AddPort(output_port, Port::OUTPUT, width);
    }
    if (klass->GetName() == resource::kEmbedded) {
      BuildEmbededResource(*res);
    }
    if (resource::IsExclusiveBinOp(*klass)) {
      BuildExclusiveBinOpResource(*res);
    }
    if (resource::IsMapped(*klass)) {
      BuildMappedResource(*res);
    }
    if (resource::IsArray(*klass)) {
      BuildArrayResource(*res);
    }
    if (resource::IsSubModuleTask(*klass)) {
      task_->BuildSubModuleTaskResource(*res);
    }
    if (resource::IsSubModuleTaskCall(*klass)) {
      BuildSubModuleTaskCallResource(*res);
    }
    if (resource::IsSiblingTask(*klass)) {
      task_->BuildSiblingTaskResource(*res);
    }
    if (resource::IsSiblingTaskCall(*klass)) {
      BuildSiblingTaskCallResource(*res);
    }
    if (resource::IsForeignRegister(*klass)) {
      BuildForeignRegister(*res);
    }
  }
}

void Table::BuildExclusiveBinOpResource(const IResource &res) {
  ostream &rs = tmpl_->GetStream(kResourceSection);
  const string &res_name = res.GetClass()->GetName();
  rs << "  // " << res_name << ":" << res.GetId() << "\n";
  map<IState *, IInsn *> callers;
  CollectResourceCallers(res, "", &callers);
  if (callers.size() == 0) {
    return;
  }
  string name = InsnWriter::ResourceName(res);
  WriteInputSel(name + "_s0", res, callers, 0, rs);
  WriteInputSel(name + "_s1", res, callers, 1, rs);
  WriteWire(name + "_d0", res.output_types_[0], rs);

  rs << "  assign " << name << + "_d0 = "
     << name + "_s0 ";
  if (res_name == resource::kGt) {
    rs << ">";
  } else if (res_name == resource::kAdd) {
    rs << "+";
  } else {
    LOG(FATAL) << "Unknown binop" << res_name;
  }
  rs << " " << name + "_s1;\n";
}

void Table::BuildArrayResource(const IResource &res) {
}

void Table::BuildMappedResource(const IResource &res) {
  auto *params = res.GetParams();
  if (params->GetMappedName() == "mem") {
    BuildSRAMResource(res);
  }
}

void Table::BuildSRAMResource(const IResource &res) {
  InternalSRAM *sram = mod_->RequestInternalSRAM(res);
  ostream &es = tmpl_->GetStream(kEmbeddedInstanceSection);
  string name = sram->GetModuleName();
  string res_id = Util::Itoa(res.GetId());
  string inst = name + "_inst_" + res_id;
  es << "  " << name << " " << inst << "("
     << ".clk(" << ports_->GetClk() << ")"
     << ", ." << sram->GetResetPinName() << "(" << ports_->GetReset() << ")"
     << ", .addr_i(sram_addr_" << res_id << ")"
     << ", .rdata_o(sram_rdata_" << res_id << ")"
     << ", .wdata_i(sram_wdata_" << res_id << ")"
     << ", .write_en_i(sram_wdata_en_" << res_id << ")"
     <<");\n";
  ostream &rs = tmpl_->GetStream(kResourceSection);
  rs << "  reg " << sram->AddressWidthSpec() << "sram_addr_" << res_id << ";\n"
     << "  wire " << sram->DataWidthSpec() << "sram_rdata_" << res_id << ";\n"
     << "  reg " << sram->DataWidthSpec() << "sram_wdata_" << res_id << ";\n"
     << "  reg sram_wdata_en_" << res_id << ";\n";
  map<IState *, IInsn *> callers;
  CollectResourceCallers(res, "sram_write", &callers);
  ostream &fs = tmpl_->GetStream(kStateOutput + Util::Itoa(table_id_));
  fs << "      sram_wdata_en_" << res_id << " <= ";
  WriteStateUnion(callers, fs);
  fs << ";\n";
}

void Table::BuildSiblingTaskCallResource(const IResource &res) {
  vector<IState *> sts;
  for (IState *st : i_table_->states_) {
    for (IInsn *insn : st->insns_) {
      if (insn->GetResource() == &res) {
	sts.push_back(st);
      }
    }
  }
  ostream &rs = tmpl_->GetStream(kResourceSection);
  const ITable *callee_tab = res.GetCalleeTable();
  rs << "  assign " << Task::TaskEnablePin(*callee_tab) << " = ";
  rs << JoinStates(sts);
  rs << ";\n";
}

void Table::BuildSubModuleTaskCallResource(const IResource &res) {
  ostream &rs = tmpl_->GetStream(kResourceSection);
  string prefix = Task::SubModuleTaskControlPinPrefix(res);
  rs << "  reg " << prefix << "_en;\n";
  rs << "  wire " << prefix << "_ack;\n";
  ostream &is = tmpl_->GetStream(kInitialValueSection + Util::Itoa(table_id_));
  is << "      " << prefix << "_en <= 0;\n";
}

void Table::BuildForeignRegister(const IResource &res) {
  vector<pair<IState *, IInsn *>> writers;
  for (IState *st : i_table_->states_) {
    for (IInsn *insn : st->insns_) {
      if (insn->GetResource() == &res) {
	if (insn->inputs_.size() > 0) {
	  writers.push_back(make_pair(st, insn));
	}
      }
    }
  }
  IRegister *foreign_reg = res.GetForeignRegister();
  string res_name = SharedRegPrefix(*i_table_, *foreign_reg);
  ostream &rs = tmpl_->GetStream(kResourceSection);
  rs << "  // " << res_name << "\n";
  rs << "  wire " << res_name << "_w;\n";
  rs << "  wire " << WidthSpec(foreign_reg) << " " << res_name << "_wdata;\n";
  if (writers.size() == 0) {
    rs << "  assign " << res_name << "_w = 0;\n";
    rs << "  assign " << res_name << "_wdata = 0;\n";
    return;
  }
  vector<IState *> sts;
  for (auto &w : writers) {
    sts.push_back(w.first);
  }
  rs << "  assign " << res_name << "_w = ";
  rs << JoinStates(sts);
  rs << ";\n";

  string d;
  for (auto &w : writers) {
    IInsn *insn = w.second;
    if (d.empty()) {
      d = InsnWriter::RegisterName(*insn->inputs_[0]);
    } else {
      IState *st = w.first;
      string t;
      t = "(" + StateVariable() + " == " + Util::Itoa(st->GetId()) + ") ? ";
      t += InsnWriter::RegisterName(*insn->inputs_[0]);
      t += " : (" + d + ")";
      d = t;
    }
  }
  rs << "  assign " << res_name << "_wdata = " << d << ";\n";
}

void Table::BuildEmbededResource(const IResource &res) {
  auto *params = res.GetParams();
  embed_->RequestModule(*params);
  ostream &is = tmpl_->GetStream(kEmbeddedInstanceSection);
  embed_->BuildModuleInstantiation(res, *ports_, is);
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

string Table::SharedRegPrefix(const ITable &writer, const IRegister &reg) {
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

void Table::CollectResourceCallers(const IResource &res,
				   const string &opr,
				   map<IState *, IInsn *> *callers) {
  for (auto *st : i_table_->states_) {
    for (auto *insn : st->insns_) {
      if (insn->GetResource() == &res &&
	  insn->GetOperand() == opr) {
	callers->insert(make_pair(st, insn));
      }
    }
  }
}

void Table::WriteWire(const string &name, const IValueType &type,
		      ostream &os) {
  os << "  wire ";
  int width = type.GetWidth();
  if (width > 0) {
    os << "[" << (width - 1) << ":0] ";
  }
  os << name << ";\n";
}

void Table::WriteInputSel(const string &name, const IResource &res,
			  const map<IState *, IInsn *> &callers,
			  int nth,
			  ostream &os) {
  WriteWire(name, res.input_types_[nth], os);
  os << "  assign " << name << " = ";
  if (callers.size() == 1) {
    IInsn *insn = (callers.begin())->second;
    os << InsnWriter::RegisterName(*insn->inputs_[nth]);
  } else {
    LOG(FATAL) << "TODO(yt76): Input selector";
  }
  os << ";\n";
}

void Table::WriteStateUnion(const map<IState *, IInsn *> &callers,
			    ostream &os) {
  if (callers.size() == 0) {
    os << "0";
  }
  bool is_first = true;
  for (auto &c : callers) {
    if (!is_first) {
      os << " | ";
    }
    os << "(" << StateVariable() << " == " << c.first->GetId() << ")";
    is_first = false;
  }
}

bool Table::IsTask() {
  return (task_.get() != nullptr);
}

bool Table::IsEmpty() {
  return (states_.size() == 0);
}

string Table::JoinStates(const vector<IState *> &sts) {
  vector<string> conds;
  for (IState *st : sts) {
    conds.push_back("(" + StateVariable() + " == " + Util::Itoa(st->GetId()) + ")");
  }
  return Util::Join(conds, " || ");
}

Ports *Table::GetPorts() const {
  return ports_;
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
