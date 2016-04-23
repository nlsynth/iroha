#include "writer/verilog/task.h"

#include "design/design_util.h"
#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "writer/module_template.h"
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

bool Task::IsTask(const Table &table) {
  ITable *i_table = table.GetITable();
  IInsn *task_entry_insn = DesignUtil::FindTaskEntryInsn(i_table);
  if (task_entry_insn != nullptr) {
    return true;
  }
  return false;
}

string Task::TaskEnablePin(const ITable &tab) {
  return "task_" + Util::Itoa(tab.GetId()) + "_en";
}

void Task::BuildSubModuleTask() {
  string en = Task::TaskEnablePin(*tab_.GetITable());
  Ports *ports = tab_.GetPorts();
  ports->AddPort(en, Port::INPUT, 0);
  int table_id = tab_.GetITable()->GetId();
  string ack = "task_" + Util::Itoa(table_id) + "_ack";
  ports->AddPort(ack, Port::OUTPUT, 0);
  ModuleTemplate *tmpl = tab_.GetModuleTemplate();
  ostream &fs = tmpl->GetStream(kStateOutput + Util::Itoa(table_id));
  fs << "      " << ack <<
    " <= (" << tab_.StateVariable() << " == `"
     << tab_.StateName(Task::kTaskEntryStateId) << ") && " << en << ";\n";
}

void Task::BuildSiblingTask() {
  ModuleTemplate *tmpl = tab_.GetModuleTemplate();
  ostream &rs = tmpl->GetStream(kResourceSection);
  rs << "  wire " << Task::TaskEnablePin(*tab_.GetITable()) << ";\n";
}

string Task::SubModuleTaskControlPinPrefix(const IResource &res) {
  return "task_" + Util::Itoa(res.GetTable()->GetId())
    + "_" + Util::Itoa(res.GetId());
}

void Task::BuildSubModuleTaskCall() {
  auto *tmpl = tab_.GetModuleTemplate();
  ostream &rs = tmpl->GetStream(kResourceSection);
  string prefix = Task::SubModuleTaskControlPinPrefix(res_);
  rs << "  reg " << prefix << "_en;\n";
  rs << "  wire " << prefix << "_ack;\n";
  ostream &is = tmpl->GetStream(kInitialValueSection + Util::Itoa(tab_.GetITable()->GetId()));
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
  auto *tmpl = tab_.GetModuleTemplate();
  ostream &rs = tmpl->GetStream(kResourceSection);
  const ITable *callee_tab = res_.GetCalleeTable();
  rs << "  assign " << Task::TaskEnablePin(*callee_tab) << " = ";
  rs << JoinStates(sts);
  rs << ";\n";
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
