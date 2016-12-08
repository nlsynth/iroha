#include "writer/verilog/sibling_task.h"

#include "design/design_util.h"
#include "iroha/i_design.h"
#include "iroha/logging.h"
#include "iroha/resource_class.h"
#include "writer/connection.h"
#include "writer/module_template.h"
#include "writer/verilog/insn_writer.h"
#include "writer/verilog/module.h"
#include "writer/verilog/state.h"
#include "writer/verilog/table.h"

namespace iroha {
namespace writer {
namespace verilog {

const int SiblingTask::kTaskEntryStateId = -1;

SiblingTask::SiblingTask(const IResource &res, const Table &table)
  : Resource(res, table) {
}

void SiblingTask::BuildResource() {
  auto *klass = res_.GetClass();
  if (resource::IsSiblingTask(*klass)) {
    BuildSiblingTask();
  }
  if (resource::IsSiblingTaskCall(*klass)) {
    BuildSiblingTaskCall();
  }
}

void SiblingTask::BuildInsn(IInsn *insn, State *st) {
  auto *klass = res_.GetClass();
  if (resource::IsSiblingTask(*klass)) {
    if (insn->GetOperand() == "") {
      BuildSiblingTaskInsn(insn, st);
    }
  }
}

string SiblingTask::ReadySignal(IInsn *insn) {
  auto *klass = res_.GetClass();
  if (resource::IsSiblingTaskCall(*klass)) {
    const ITable *callee_tab = res_.GetCalleeTable();
    if (insn->GetOperand() == "wait") {
      // Wait for the callee resource's availability.
      return "!" + SiblingTaskReadySignal(*callee_tab, nullptr);
    } else {
      return SiblingTaskReadySignal(*callee_tab, res_.GetTable());
    }
  }
  return "";
}

bool SiblingTask::IsTask(const Table &table) {
  ITable *i_table = table.GetITable();
  IInsn *task_entry_insn = DesignUtil::FindTaskEntryInsn(i_table);
  if (task_entry_insn != nullptr) {
    return true;
  }
  return false;
}

bool SiblingTask::IsSiblingTask(const Table &table) {
  ITable *i_table = table.GetITable();
  IInsn *task_entry_insn = DesignUtil::FindTaskEntryInsn(i_table);
  if (task_entry_insn != nullptr) {
    if (resource::IsSiblingTask(*task_entry_insn->GetResource()->GetClass())) {
      return true;
    }
  }
  return false;
}

string SiblingTask::TaskEnablePin(const ITable &tab, const ITable *caller) {
  string s = "task_" + Util::Itoa(tab.GetId());
  if (caller != nullptr) {
    s += "_" + Util::Itoa(caller->GetId());
  }
  return s + "_en";
}

void SiblingTask::BuildSiblingTask() {
  vector<IResource *> callers;
  ITable *i_tab = tab_.GetITable();
  for (ITable *other_tab : i_tab->GetModule()->tables_) {
    for (auto *caller_res : other_tab->resources_) {
      if (caller_res->GetCalleeTable() == i_tab) {
	callers.push_back(caller_res);
      }
    }
  }
  if (callers.size() == 0) {
    LOG(INFO) << "No callers for this task:" << i_tab->GetId();
    return;
  }
  ostream &rs = tmpl_->GetStream(kResourceSection);
  rs << "  //  Sibling task: " << i_tab->GetId()
     << ":" << res_.GetId() << "\n";

  // Request wires from callers.
  string callee_pin = TaskEnablePin(*i_tab, nullptr);
  vector<string> caller_pins;
  for (IResource *caller : callers) {
    string caller_pin = TaskEnablePin(*tab_.GetITable(), caller->GetTable());
    // will be assigned by the caller.
    rs << "  wire " << caller_pin << ";\n";
    caller_pins.push_back(caller_pin);
  }
  rs << "  wire " << callee_pin << ";\n";
  rs << "  assign " << callee_pin << " = "
     << Util::Join(caller_pins, " || ") << ";\n";

  // Acknowledge from callee.
  string callee_ready = SiblingTaskReadySignal(*i_tab, nullptr);
  rs << "  wire " << callee_ready << ";\n";
  rs << "  assign " << callee_ready
     << " = ("
     << tab_.StateVariable() << " == `"
     << tab_.StateName(SiblingTask::kTaskEntryStateId) << ");\n";
  vector<string> higher_callers;
  for (IResource *caller : callers) {
    string caller_ready =
      SiblingTaskReadySignal(*tab_.GetITable(), caller->GetTable());
    rs << "  wire " << caller_ready << ";\n";
    string wait_req;
    if (callers.size() > 1) {
      // Needs arbitration.
      wait_req = " && " + TaskEnablePin(*i_tab, caller->GetTable());
    }
    string has_higher = Util::Join(higher_callers, " || ");
    if (!has_higher.empty()) {
      has_higher = " && !(" + has_higher + ")";
    }
    rs << "  assign " << caller_ready << " = "
       << callee_ready << wait_req << has_higher << ";\n";
    higher_callers.push_back(TaskEnablePin(*i_tab,
					   caller->GetTable()));
  }

  // Arguments.
  for (int i = 0; i < res_.input_types_.size(); ++i) {
    auto &type = res_.input_types_[i];
    rs << "  reg " << Table::WidthSpec(type) << " "
       << ArgSignal(*i_tab, i, nullptr) << ";\n";
  }

  rs << "  //  sibling task end\n";

  ostream &es = tab_.TaskEntrySectionStream();
  es << "            // capture arguments\n";
  int first_caller = true;
  bool multi_callers = (callers.size() > 1);
  for (IResource *caller : callers) {
    if (multi_callers) {
      if (!first_caller) {
	es << "            else\n";
      }
      string caller_ready =
	SiblingTaskReadySignal(*tab_.GetITable(), caller->GetTable());
      es << "            if (" << caller_ready << ") begin\n";
    }
    for (int i = 0; i < res_.input_types_.size(); ++i) {
      es << "            "
	 << ArgSignal(*i_tab, i, nullptr) << " <= "
	 << ArgSignal(*i_tab, i, caller->GetTable()) << ";\n";
    }
    if (multi_callers) {
      es << "            end\n";
    }
    first_caller = false;
  }
}

void SiblingTask::BuildSiblingTaskInsn(IInsn *insn, State *st) {
  CHECK(insn->outputs_.size() == insn->GetResource()->input_types_.size())
    << "Task argument numbers don't match.";
  ostream &os = st->StateBodySectionStream();
  ostream &ws = tmpl_->GetStream(kInsnWireValueSection);
  static const char I[] = "          ";
  for (int i = 0; i < insn->outputs_.size(); ++i) {
    IRegister *lhs = insn->outputs_[i];
    if (lhs->IsStateLocal()) {
      ws << "  assign " << InsnWriter::RegisterValue(*lhs, tab_.GetNames())
	 << " = " << ArgSignal(*tab_.GetITable(), i, nullptr) << ";\n";
    } else {
      os << I << InsnWriter::RegisterValue(*lhs, tab_.GetNames())
	 << " <= " << ArgSignal(*tab_.GetITable(), i, nullptr) << ";\n";
    }
  }
}

void SiblingTask::BuildSiblingTaskCall() {
  map<IState *, IInsn *> callers;
  CollectResourceCallers("", &callers);

  ostream &rs = tmpl_->GetStream(kResourceSection);
  const ITable *callee_tab = res_.GetCalleeTable();
  rs << "  assign " << TaskEnablePin(*callee_tab, tab_.GetITable()) << " = ";
  // TODO(yt76): This doesn't work for compound state. A task may finish
  // and unintentionally start again while waiting for other insns...
  // (maybe (st == s_n && sub_st == 0) ?)
  rs << JoinStates(callers);
  rs << ";\n";

  CollectResourceCallers("", &callers);
  for (int i = 0; i < res_.input_types_.size(); ++i) {
    WriteInputSel(ArgSignal(*callee_tab, i, res_.GetTable()), callers, i, rs);
  }
}

string SiblingTask::SiblingTaskReadySignal(const ITable &tab, const ITable *caller) {
  string s = "task_" + Util::Itoa(tab.GetId());
  if (caller != nullptr) {
    s += "_" + Util::Itoa(caller->GetId());
  }
  return s + "_ready";
}

string SiblingTask::ArgSignal(const ITable &tab, int nth,
			      const ITable *caller) {
  string s = "arg_" + Util::Itoa(tab.GetId()) + "_" + Util::Itoa(nth);
  if (caller != nullptr) {
    s += "_" + Util::Itoa(caller->GetId());
  }
  return s;
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha