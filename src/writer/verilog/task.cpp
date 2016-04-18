#include "writer/verilog/task.h"

#include "design/design_util.h"
#include "iroha/i_design.h"
#include "writer/module_template.h"
#include "writer/verilog/module.h"
#include "writer/verilog/ports.h"
#include "writer/verilog/table.h"

namespace iroha {
namespace writer {
namespace verilog {

const int Task::kTaskEntryStateId = -1;

Task::Task(Table *table, IInsn *insn)
  : table_(table), task_entry_insn_(insn) {
}

Task *Task::MayCreateTask(Table *table) {
  ITable *i_table = table->GetITable();
  IInsn *task_entry_insn = DesignUtil::FindTaskEntryInsn(i_table);
  if (task_entry_insn != nullptr) {
    return new Task(table, task_entry_insn);
  }
  return nullptr;
}

string Task::TaskEnablePin(const ITable &tab) {
  return "task_" + Util::Itoa(tab.GetId()) + "_en";
}

void Task::BuildSubModuleTaskResource(const IResource &res) {
  string en = Task::TaskEnablePin(*table_->GetITable());
  Ports *ports = table_->GetPorts();
  ports->AddPort(en, Port::INPUT, 0);
  int table_id = table_->GetITable()->GetId();
  string ack = "task_" + Util::Itoa(table_id) + "_ack";
  ports->AddPort(ack, Port::OUTPUT, 0);
  ModuleTemplate *tmpl = table_->GetModuleTemplate();
  ostream &fs = tmpl->GetStream(kStateOutput + Util::Itoa(table_id));
  fs << "      " << ack <<
    " <= (" << table_->StateVariable() << " == `"
     << table_->StateName(Task::kTaskEntryStateId) << ") && " << en << ";\n";
}

void Task::BuildSiblingTaskResource(const IResource &res) {
  ModuleTemplate *tmpl = table_->GetModuleTemplate();
  ostream &rs = tmpl->GetStream(kResourceSection);
  rs << "  wire " << Task::TaskEnablePin(*table_->GetITable()) << ";\n";
}

string Task::SubModuleTaskControlPinPrefix(const IResource &res) {
  return "task_" + Util::Itoa(res.GetTable()->GetId())
    + "_" + Util::Itoa(res.GetId());
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
