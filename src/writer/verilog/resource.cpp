#include "writer/verilog/resource.h"

#include "iroha/i_design.h"
#include "iroha/logging.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"
#include "writer/module_template.h"
#include "writer/verilog/channel.h"
#include "writer/verilog/embed.h"
#include "writer/verilog/ext_io.h"
#include "writer/verilog/insn_writer.h"
#include "writer/verilog/mapped.h"
#include "writer/verilog/module.h"
#include "writer/verilog/operator.h"
#include "writer/verilog/port_io.h"
#include "writer/verilog/ports.h"
#include "writer/verilog/shared_reg.h"
#include "writer/verilog/state.h"
#include "writer/verilog/sub_module_task.h"
#include "writer/verilog/table.h"
#include "writer/verilog/task.h"

namespace iroha {
namespace writer {
namespace verilog {

Resource *Resource::Create(const IResource &res, const Table &table) {
  auto *klass = res.GetClass();
  if (resource::IsSubModuleTask(*klass) ||
      resource::IsSubModuleTaskCall(*klass)) {
    return new SubModuleTask(res, table);
  }
  if (resource::IsSubModuleTask(*klass) ||
      resource::IsSubModuleTaskCall(*klass)) {
    return new Task(res, table);
  }
  if (resource::IsExclusiveBinOp(*klass) ||
      resource::IsLightBinOp(*klass) ||
      resource::IsLightUniOp(*klass) ||
      resource::IsBitShiftOp(*klass) ||
      resource::IsBitSel(*klass) ||
      resource::IsBitConcat(*klass) ||
      resource::IsSelect(*klass)) {
    return new Operator(res, table);
  }
  if (resource::IsForeignRegister(*klass)) {
    return new SharedReg(res, table);
  }
  if (resource::IsEmbedded(*klass)) {
    return new EmbeddedResource(res, table);
  }
  if (resource::IsChannelRead(*klass) ||
      resource::IsChannelWrite(*klass)) {
    return new Channel(res, table);
  }
  if (resource::IsExtInput(*klass) ||
      resource::IsExtOutput(*klass)) {
    return new ExtIO(res, table);
  }
  if (resource::IsPortInput(*klass) ||
      resource::IsPortOutput(*klass)) {
    return new PortIO(res, table);
  }
  if (resource::IsMapped(*klass)) {
    return new MappedResource(res, table);
  }
  return new Resource(res, table);
}

Resource::Resource(const IResource &res, const Table &table)
  : res_(res), tab_(table) {
  tmpl_ = tab_.GetModuleTemplate();
}

void Resource::BuildResource() {
}

void Resource::BuildInsn(IInsn *insn, State *st) {
  ostream &os = st->StateBodySectionStream();
  auto *rc = res_.GetClass();
  if (resource::IsSet(*rc)) {
    if (insn->outputs_[0]->IsStateLocal()) {
      ostream &ws = tmpl_->GetStream(kInsnWireValueSection);
      ws << "  assign " << insn->outputs_[0]->GetName() << " = "
	 << InsnWriter::RegisterName(*insn->inputs_[0]) << ";\n";
    } else {
      os << "          " << insn->outputs_[0]->GetName() << " <= "
	 << InsnWriter::RegisterName(*insn->inputs_[0]) << ";\n";
    }
    return;
  }
  InsnWriter writer(insn, st, os);
  const string &rc_name = rc->GetName();
  if (rc_name == resource::kPrint) {
    writer.Print();
  } else if (rc_name == resource::kAssert) {
    writer.Assert();
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

string Resource::SelectValueByState(int default_value) {
  string v = Util::Itoa(default_value);
  map<IState *, IInsn *> callers;
  CollectResourceCallers("", &callers);
  for (auto &c : callers) {
    IState *st = c.first;
    IInsn *insn = c.second;
    v = "((" + tab_.StateVariable() + " == " +
      Util::Itoa(st->GetId()) +
      ") ? " + InsnWriter::RegisterName(*insn->inputs_[0]) +
      " : " + v + ")";
  }
  return v;
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
