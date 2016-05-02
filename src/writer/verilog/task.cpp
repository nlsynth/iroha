#include "writer/verilog/task.h"

#include "design/design_util.h"
#include "iroha/i_design.h"
#include "iroha/logging.h"
#include "iroha/resource_class.h"
#include "writer/module_template.h"
#include "writer/verilog/insn_writer.h"
#include "writer/verilog/module.h"
#include "writer/verilog/ports.h"
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

void Task::WriteInsn(const IInsn *insn, State *st, ostream &os) {
  auto *klass = res_.GetClass();
  if (resource::IsSubModuleTaskCall(*klass)) {
    static const char I[] = "          ";
    string st = InsnWriter::MultiCycleStateName(*insn);
    IResource *res = insn->GetResource();
    string pin = Task::SubModuleTaskControlPinPrefix(*res);
    os << I << "if (" << st << " == 0) begin\n"
       << I << "  if (" << pin << "_ack) begin\n"
       << I << "    " << pin << "_en <= 0;\n"
       << I << "    " << st << " <= 3;\n"
       << I << "  end else begin\n"
       << I << "  " << pin << "_en <= 1;\n"
       << I << "  end\n"
       << I << "end\n";
  }
}

string Task::ReadySignal() {
  auto *klass = res_.GetClass();
  if (resource::IsSiblingTaskCall(*klass)) {
    const ITable *callee_tab = res_.GetCalleeTable();
    return SiblingTaskReadySignal(*callee_tab);
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
  Ports *ports = tab_.GetPorts();
  ports->AddPort(en, Port::INPUT, 0);
  int table_id = tab_.GetITable()->GetId();
  string ack = "task_" + Util::Itoa(table_id) + "_ack";
  ports->AddPort(ack, Port::OUTPUT, 0);
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
  string callee_pin = TaskEnablePin(*tab_.GetITable(), nullptr);

  vector<string> caller_pins;
  for (IResource *caller : callers) {
    string caller_pin = TaskEnablePin(*tab_.GetITable(), caller->GetTable());
    // will be assigned by the caller.
    rs << "  wire " << caller_pin << ";\n";
    caller_pins.push_back(caller_pin);
  }
  rs << "  wire " << callee_pin << ";\n";
  rs << "  assign " << callee_pin << " = " << Util::Join(caller_pins, " || ") << ";\n";

  rs << "  wire " << SiblingTaskReadySignal(*tab_.GetITable()) << ";\n";
  rs << "  assign " << SiblingTaskReadySignal(*tab_.GetITable())
     << " = ("
     << tab_.StateVariable() << " == `"
     << tab_.StateName(Task::kTaskEntryStateId) << ");\n";
}

string Task::SubModuleTaskControlPinPrefix(const IResource &res) {
  return "task_" + Util::Itoa(res.GetTable()->GetId())
    + "_" + Util::Itoa(res.GetId());
}

void Task::BuildSubModuleTaskCall() {
  ostream &rs = tmpl_->GetStream(kResourceSection);
  string prefix = Task::SubModuleTaskControlPinPrefix(res_);
  rs << "  reg " << prefix << "_en;\n";
  rs << "  wire " << prefix << "_ack;\n";
  ostream &is = tab_.InitialValueSectionStream();
  is << "      " << prefix << "_en <= 0;\n";
}

void Task::BuildSiblingTaskCall() {
  vector<IState *> sts;
  for (IState *st : tab_.GetITable()->states_) {
    for (IInsn *insn : st->insns_) {
      if (insn->GetResource() == &res_) {
	sts.push_back(st);
      }
    }
  }
  ostream &rs = tmpl_->GetStream(kResourceSection);
  const ITable *callee_tab = res_.GetCalleeTable();
  rs << "  assign " << TaskEnablePin(*callee_tab, tab_.GetITable()) << " = ";
  rs << JoinStates(sts);
  rs << ";\n";
}

string Task::SiblingTaskReadySignal(const ITable &tab) {
  return "task_" + Util::Itoa(tab.GetId()) + "_ready";
}
  
}  // namespace verilog
}  // namespace writer
}  // namespace iroha
