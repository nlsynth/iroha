#include "writer/verilog/task.h"

#include "design/design_util.h"
#include "iroha/i_design.h"
#include "iroha/logging.h"
#include "iroha/resource_class.h"
#include "writer/connection.h"
#include "writer/module_template.h"
#include "writer/verilog/insn_writer.h"
#include "writer/verilog/module.h"
#include "writer/verilog/ports.h"
#include "writer/verilog/state.h"
#include "writer/verilog/table.h"

namespace iroha {
namespace writer {
namespace verilog {

const int Task::kTaskEntryStateId = -1;

Task::Task(const IResource &res, const Table &table)
  : Resource(res, table) {
}

void Task::BuildResource() {
  auto *klass = res_.GetClass();
  if (resource::IsSubModuleTask(*klass)) {
    BuildSubModuleTask();
  }
  if (resource::IsSiblingTask(*klass)) {
    BuildSiblingTask();
  }
  if (resource::IsSubModuleTaskCall(*klass)) {
    BuildSubModuleTaskCall();
  }
  if (resource::IsSiblingTaskCall(*klass)) {
    BuildSiblingTaskCall();
  }
}

void Task::BuildInsn(IInsn *insn, State *st) {
  auto *klass = res_.GetClass();
  if (resource::IsSubModuleTaskCall(*klass)) {
    BuildSubModuleTaskCallInsn(insn, st);
  }
  if (resource::IsSiblingTask(*klass)) {
    if (insn->GetOperand() == "") {
      BuildSiblingTaskInsn(insn, st);
    }
  }
}

string Task::ReadySignal(IInsn *insn) {
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
  if (resource::IsSubModuleTaskCall(*klass)) {
    string pin = SubModuleTaskControlPinPrefix(res_);
    return pin + "_ack";
  }
  return "";
}

bool Task::IsTask(const Table &table) {
  ITable *i_table = table.GetITable();
  IInsn *task_entry_insn = DesignUtil::FindTaskEntryInsn(i_table);
  if (task_entry_insn != nullptr) {
    return true;
  }
  return false;
}

string Task::TaskEnablePin(const ITable &tab, const ITable *caller) {
  string s = "task_" + Util::Itoa(tab.GetId());
  if (caller != nullptr) {
    s += "_" + Util::Itoa(caller->GetId());
  }
  return s + "_en";
}

void Task::BuildSubModuleTask() {
  string en = Task::TaskEnablePin(*tab_.GetITable(), nullptr);
  int table_id = tab_.GetITable()->GetId();
  string ack = "task_" + Util::Itoa(table_id) + "_ack";
  ostream &fs = tab_.StateOutputSectionStream();
  fs << "      " << ack <<
    " <= (" << tab_.StateVariable() << " == `"
     << tab_.StateName(Task::kTaskEntryStateId) << ") && " << en << ";\n";
}

void Task::BuildSiblingTask() {
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
     << tab_.StateName(Task::kTaskEntryStateId) << ");\n";
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

void Task::BuildSiblingTaskInsn(IInsn *insn, State *st) {
  CHECK(insn->outputs_.size() == insn->GetResource()->input_types_.size())
    << "Task argument numbers don't match.";
  ostream &os = st->StateBodySectionStream();
  ostream &ws = tmpl_->GetStream(kInsnWireValueSection);
  static const char I[] = "          ";
  for (int i = 0; i < insn->outputs_.size(); ++i) {
    IRegister *lhs = insn->outputs_[i];
    if (lhs->IsStateLocal()) {
      ws << "  assign " << InsnWriter::RegisterName(*lhs)
	 << " = " << ArgSignal(*tab_.GetITable(), i, nullptr) << ";\n";
    } else {
      os << I << InsnWriter::RegisterName(*lhs)
	 << " <= " << ArgSignal(*tab_.GetITable(), i, nullptr) << ";\n";
    }
  }
}

void Task::BuildSubModuleTaskCallInsn(IInsn *insn, State *st) {
  map<IState *, IInsn *> callers;
  CollectResourceCallers("", &callers);

  ostream &os = st->StateBodySectionStream();
  static const char I[] = "          ";
  string st_name = InsnWriter::MultiCycleStateName(*(insn->GetResource()));
  IResource *res = insn->GetResource();
  string pin = SubModuleTaskControlPinPrefix(*res);
  os << I << "if (" << st_name << " == 0) begin\n"
     << I << "  if (" << pin << "_ack) begin\n"
     << I << "    " << st_name << " <= 3;\n"
     << I << "  end else begin\n"
     << I << "  end\n"
     << I << "end\n";
}

string Task::SubModuleTaskControlPinPrefix(const IResource &res) {
  return "task_" + Util::Itoa(res.GetTable()->GetId())
    + "_" + Util::Itoa(res.GetId());
}

void Task::BuildSubModuleTaskCall() {
  map<IState *, IInsn *> callers;
  CollectResourceCallers("", &callers);

  ostream &rs = tmpl_->GetStream(kResourceSection);
  string prefix = Task::SubModuleTaskControlPinPrefix(res_);
  rs << "  wire " << prefix << "_en;\n";
  rs << "  wire " << prefix << "_ack;\n";
  rs << "  assign " << prefix << "_en = "
     << JoinStatesWithSubState(callers, 0) << ";\n";
}

void Task::BuildSiblingTaskCall() {
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

string Task::SiblingTaskReadySignal(const ITable &tab, const ITable *caller) {
  string s = "task_" + Util::Itoa(tab.GetId());
  if (caller != nullptr) {
    s += "_" + Util::Itoa(caller->GetId());
  }
  return s + "_ready";
}

string Task::ArgSignal(const ITable &tab, int nth, const ITable *caller) {
  string s = "arg_" + Util::Itoa(tab.GetId()) + "_" + Util::Itoa(nth);
  if (caller != nullptr) {
    s += "_" + Util::Itoa(caller->GetId());
  }
  return s;
}

void Task::BuildChildTaskWire(const TaskCallInfo &ti,
			      ostream &is) {
  for (IResource *caller : ti.tasks_) {
    ITable *tab = caller->GetTable();
    string caller_en;
    string caller_ack;
    string prefix = Task::SubModuleTaskControlPinPrefix(*caller);
    caller_en = prefix + "_en";
    caller_ack = prefix + "_ack";
    is << ", .task_" << tab->GetId() << "_en(" << caller_en << ")";
    is << ", .task_" << tab->GetId() << "_ack(" << caller_ack << ")";
  }
}

void Task::BuildPorts(const TaskCallInfo &ti, Ports *ports) {
  for (IResource *caller : ti.tasks_) {
    ITable *callee_tab = caller->GetCalleeTable();
    string en = Task::TaskEnablePin(*callee_tab, nullptr);
    ports->AddPort(en, Port::INPUT, 0);
    int table_id = callee_tab->GetId();
    string ack = "task_" + Util::Itoa(table_id) + "_ack";
    ports->AddPort(ack, Port::OUTPUT, 0);
  }
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
