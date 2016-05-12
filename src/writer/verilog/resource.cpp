#include "writer/verilog/resource.h"

#include "iroha/i_design.h"
#include "iroha/logging.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"
#include "writer/module_template.h"
#include "writer/verilog/embed.h"
#include "writer/verilog/insn_writer.h"
#include "writer/verilog/internal_sram.h"
#include "writer/verilog/module.h"
#include "writer/verilog/operator.h"
#include "writer/verilog/ports.h"
#include "writer/verilog/shared_reg.h"
#include "writer/verilog/state.h"
#include "writer/verilog/table.h"
#include "writer/verilog/task.h"

namespace iroha {
namespace writer {
namespace verilog {

Resource *Resource::Create(const IResource &res, const Table &table) {
  auto *klass = res.GetClass();
  if (resource::IsSubModuleTask(*klass) ||
      resource::IsSiblingTask(*klass) ||
      resource::IsSubModuleTaskCall(*klass) ||
      resource::IsSiblingTaskCall(*klass)) {
    return new Task(res, table);
  }
  if (resource::IsExclusiveBinOp(*klass) ||
      resource::IsLightBinOp(*klass) ||
      resource::IsBitArrangeOp(*klass)) {
    return new Operator(res, table);
  }
  if (resource::IsForeignRegister(*klass)) {
    return new SharedReg(res, table);
  }
  if (resource::IsEmbedded(*klass)) {
    return new EmbeddedResource(res, table);
  }
  return new Resource(res, table);
}

Resource::Resource(const IResource &res, const Table &table)
  : res_(res), tab_(table) {
  tmpl_ = tab_.GetModuleTemplate();
}

void Resource::BuildResource() {
  auto *klass = res_.GetClass();
  auto *params = res_.GetParams();
  if (klass->GetName() == resource::kExtInput) {
    auto *ports = tab_.GetPorts();
    string input_port;
    int width;
    params->GetExtInputPort(&input_port, &width);
    ports->AddPort(input_port, Port::INPUT, width);
  }
  if (klass->GetName() == resource::kExtOutput) {
    auto *ports = tab_.GetPorts();
    string output_port;
    int width;
    params->GetExtOutputPort(&output_port, &width);
    ports->AddPort(output_port, Port::OUTPUT, width);
  }
  if (resource::IsMapped(*klass)) {
    BuildMapped();
  }
  if (resource::IsArray(*klass)) {
    BuildArray();
  }
}

void Resource::BuildInsn(IInsn *insn, State *st) {
  auto *klass = res_.GetClass();
  if (resource::IsExtInput(*klass)) {
    BuildExtInputInsn(insn);
  }
  if (resource::IsMapped(*klass)) {
    BuildMappedInsn(insn);
  }

  ostream &os = st->StateBodySectionStream();
  InsnWriter writer(insn, st, os);
  auto *rc = res_.GetClass();
  const string &rc_name = rc->GetName();
  if (resource::IsSet(*rc)) {
    writer.Set();
  } else if (rc_name == resource::kExtOutput) {
    writer.ExtOutput();
  } else if (rc_name == resource::kPrint) {
    writer.Print();
  } else if (rc_name == resource::kAssert) {
    writer.Assert();
  } else if (resource::IsMapped(*rc)) {
    writer.Mapped();
  }
}

string Resource::ReadySignal(IInsn *insn) {
  return "";
}

void Resource::CollectResourceCallers(const string &opr,
				      map<IState *, IInsn *> *callers) {
  for (auto *st : res_.GetTable()->states_) {
    for (auto *insn : st->insns_) {
      if (insn->GetResource() == &res_ &&
	  insn->GetOperand() == opr) {
	callers->insert(make_pair(st, insn));
      }
    }
  }
}

void Resource::WriteInputSel(const string &name,
			     const map<IState *, IInsn *> &callers,
			     int nth,
			     ostream &os) {
  WriteWire(name, res_.input_types_[nth], os);
  os << "  assign " << name << " = ";

  string cond;
  for (auto &it : callers) {
    IInsn *insn = it.second;
    IState *st = it.first;
    if (cond.empty()) {
      cond = InsnWriter::RegisterName(*insn->inputs_[nth]);
    } else {
      cond = "(" + tab_.StateVariable() + " == `" +
	tab_.StateName(st->GetId()) + ") ? " +
	InsnWriter::RegisterName(*insn->inputs_[nth]) + " : (" + cond + ")";
    }
  }
  os << cond << ";\n";
}

void Resource::WriteWire(const string &name, const IValueType &type,
			 ostream &os) {
  os << "  wire ";
  if (type.IsSigned()) {
    os << "signed ";
  }
  int width = type.GetWidth();
  if (width > 0) {
    os << "[" << (width - 1) << ":0] ";
  }
  os << name << ";\n";
}

void Resource::BuildMapped() {
  auto *params = res_.GetParams();
  if (params->GetMappedName() == "mem") {
    BuildSRAM();
  }
}

void Resource::BuildSRAM() {
  InternalSRAM *sram =
    tab_.GetEmbeddedModules()->RequestInternalSRAM(*tab_.GetModule(), res_);
  auto *ports = tab_.GetPorts();
  ostream &es = tmpl_->GetStream(kEmbeddedInstanceSection);
  string name = sram->GetModuleName();
  string res_id = Util::Itoa(res_.GetId());
  string inst = name + "_inst_" + res_id;
  es << "  " << name << " " << inst << "("
     << ".clk(" << ports->GetClk() << ")"
     << ", ." << sram->GetResetPinName() << "(" << ports->GetReset() << ")"
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
  CollectResourceCallers("sram_write", &callers);
  ostream &fs = tab_.StateOutputSectionStream();
  fs << "      sram_wdata_en_" << res_id << " <= ";
  WriteStateUnion(callers, fs);
  fs << ";\n";
}

void Resource::WriteStateUnion(const map<IState *, IInsn *> &callers,
			       ostream &os) {
  if (callers.size() == 0) {
    os << "0";
  }
  bool is_first = true;
  for (auto &c : callers) {
    if (!is_first) {
      os << " | ";
    }
    os << "(" << tab_.StateVariable() << " == " << c.first->GetId() << ")";
    is_first = false;
  }
}

void Resource::BuildArray() {
}

string Resource::JoinStates(const map<IState *, IInsn *> &sts) {
  vector<string> conds;
  for (auto &p : sts) {
    IState *st = p.first;
    conds.push_back("(" + tab_.StateVariable() + " == `" +
		    Table::StateNameFromTable(*tab_.GetITable(), st->GetId()) +
		    ")");
  }
  return Util::Join(conds, " || ");
}

string Resource::JoinStatesWithSubState(const map<IState *, IInsn *> &sts,
					int sub) {
  vector<string> conds;
  for (auto &p : sts) {
    IState *st = p.first;
    IInsn *insn = p.second;
    conds.push_back("(" + tab_.StateVariable() + " == `" +
		    Table::StateNameFromTable(*tab_.GetITable(), st->GetId()) +
		    " && " +
		    InsnWriter::MultiCycleStateName(*insn->GetResource()) +
		    " == " + Util::Itoa(sub) +
		    ")");
  }
  return Util::Join(conds, " || ");
}

void Resource::BuildExtInputInsn(IInsn *insn) {
  auto *params = res_.GetParams();
  string input_port;
  int width;
  params->GetExtInputPort(&input_port, &width);
  ostream &ws = tmpl_->GetStream(kInsnWireValueSection);
  ws << "  assign " << InsnWriter::InsnOutputWireName(*insn, 0)
     << " = "
     << input_port << ";\n";
}

void Resource::BuildMappedInsn(IInsn *insn) {
  IResource *res = insn->GetResource();
  auto *params = res->GetParams();
  ostream &ws = tmpl_->GetStream(kInsnWireValueSection);
  if (params->GetMappedName() == "mem") {
    string res_id = Util::Itoa(res->GetId());
    const string &opr = insn->GetOperand();
    if (opr == "sram_read_data") {
      ws << "  assign " << InsnWriter::InsnOutputWireName(*insn, 0)
	 << " = sram_rdata_" << res_id << ";\n";
    }
  }
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
