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
#include "writer/verilog/ports.h"
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
  return new Resource(res, table);
}

Resource::Resource(const IResource &res, const Table &table)
  : res_(res), tab_(table) {
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
  if (klass->GetName() == resource::kEmbedded) {
    BuildEmbedded();
  }
  if (resource::IsExclusiveBinOp(*klass)) {
    BuildExclusiveBinOp();
  }
  if (resource::IsMapped(*klass)) {
    BuildMapped();
  }
  if (resource::IsArray(*klass)) {
    BuildArray();
  }
  if (resource::IsForeignRegister(*klass)) {
    BuildForeignRegister();
  }
}

void Resource::BuildEmbedded() {
  auto *params = res_.GetParams();
  auto *ports = tab_.GetPorts();
  auto *embed = tab_.GetEmbed();
  auto *tmpl = tab_.GetModuleTemplate();
  embed->RequestModule(*params);
  ostream &is = tmpl->GetStream(kEmbeddedInstanceSection);
  embed->BuildModuleInstantiation(res_, *ports, is);
}

void Resource::BuildExclusiveBinOp() {
  auto *tmpl = tab_.GetModuleTemplate();
  ostream &rs = tmpl->GetStream(kResourceSection);
  const string &res_name = res_.GetClass()->GetName();
  rs << "  // " << res_name << ":" << res_.GetId() << "\n";
  map<IState *, IInsn *> callers;
  CollectResourceCallers("", &callers);
  if (callers.size() == 0) {
    return;
  }
  string name = InsnWriter::ResourceName(res_);
  WriteInputSel(name + "_s0", callers, 0, rs);
  WriteInputSel(name + "_s1", callers, 1, rs);
  WriteWire(name + "_d0", res_.output_types_[0], rs);

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
  if (callers.size() == 1) {
    IInsn *insn = (callers.begin())->second;
    os << InsnWriter::RegisterName(*insn->inputs_[nth]);
  } else {
    LOG(FATAL) << "TODO(yt76): Input selector";
  }
  os << ";\n";
}

void Resource::WriteWire(const string &name, const IValueType &type,
			 ostream &os) {
  os << "  wire ";
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
  InternalSRAM *sram = tab_.GetModule()->RequestInternalSRAM(res_);
  auto *tmpl = tab_.GetModuleTemplate();
  auto *ports = tab_.GetPorts();
  ostream &es = tmpl->GetStream(kEmbeddedInstanceSection);
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
  ostream &rs = tmpl->GetStream(kResourceSection);
  rs << "  reg " << sram->AddressWidthSpec() << "sram_addr_" << res_id << ";\n"
     << "  wire " << sram->DataWidthSpec() << "sram_rdata_" << res_id << ";\n"
     << "  reg " << sram->DataWidthSpec() << "sram_wdata_" << res_id << ";\n"
     << "  reg sram_wdata_en_" << res_id << ";\n";
  map<IState *, IInsn *> callers;
  CollectResourceCallers("sram_write", &callers);
  ostream &fs = tmpl->GetStream(kStateOutput + Util::Itoa(tab_.GetITable()->GetId()));
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

void Resource::BuildForeignRegister() {
  vector<pair<IState *, IInsn *>> writers;
  for (IState *st : res_.GetTable()->states_) {
    for (IInsn *insn : st->insns_) {
      if (insn->GetResource() == &res_) {
	if (insn->inputs_.size() > 0) {
	  writers.push_back(make_pair(st, insn));
	}
      }
    }
  }
  IRegister *foreign_reg = res_.GetForeignRegister();
  string res_name = tab_.SharedRegPrefix(*tab_.GetITable(), *foreign_reg);
  auto *tmpl = tab_.GetModuleTemplate();
  ostream &rs = tmpl->GetStream(kResourceSection);
  rs << "  // " << res_name << "\n";
  rs << "  wire " << res_name << "_w;\n";
  rs << "  wire " << Table::WidthSpec(foreign_reg) << " " << res_name << "_wdata;\n";
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
      t = "(" + tab_.StateVariable() + " == " + Util::Itoa(st->GetId()) + ") ? ";
      t += InsnWriter::RegisterName(*insn->inputs_[0]);
      t += " : (" + d + ")";
      d = t;
    }
  }
  rs << "  assign " << res_name << "_wdata = " << d << ";\n";
}

string Resource::JoinStates(const vector<IState *> &sts) {
  vector<string> conds;
  for (IState *st : sts) {
    conds.push_back("(" + tab_.StateVariable() + " == " + Util::Itoa(st->GetId()) + ")");
  }
  return Util::Join(conds, " || ");
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
