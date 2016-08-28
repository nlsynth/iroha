#include "writer/verilog/sub_module_task.h"

#include "iroha/i_design.h"
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

SubModuleTask::SubModuleTask(const IResource &res, const Table &table)
  : Task(res, table) {
}

void SubModuleTask::BuildResource() {
  auto *klass = res_.GetClass();
  if (resource::IsSubModuleTask(*klass)) {
    BuildSubModuleTask();
  }
  if (resource::IsSubModuleTaskCall(*klass)) {
    BuildSubModuleTaskCall();
  }
}

void SubModuleTask::BuildInsn(IInsn *insn, State *st) {
  auto *klass = res_.GetClass();
  if (resource::IsSubModuleTaskCall(*klass)) {
    BuildSubModuleTaskCallInsn(insn, st);
  }
}

string SubModuleTask::ReadySignal(IInsn *insn) {
  auto *klass = res_.GetClass();
  if (resource::IsSubModuleTaskCall(*klass)) {
    string pin = SubModuleTaskControlPinPrefix(res_);
    return pin + "_ack";
  }
  return "";
}

void SubModuleTask::BuildSubModuleTask() {
  string en = Task::TaskEnablePin(*tab_.GetITable(), nullptr);
  int table_id = tab_.GetITable()->GetId();
  string ack = "task_" + Util::Itoa(table_id) + "_ack";
  ostream &fs = tab_.StateOutputSectionStream();
  fs << "      " << ack <<
    " <= (" << tab_.StateVariable() << " == `"
     << tab_.StateName(Task::kTaskEntryStateId) << ") && " << en << ";\n";
}

void SubModuleTask::BuildSubModuleTaskCall() {
  map<IState *, IInsn *> callers;
  CollectResourceCallers("", &callers);

  ostream &rs = tmpl_->GetStream(kResourceSection);
  string prefix = SubModuleTaskControlPinPrefix(res_);
  rs << "  wire " << prefix << "_en;\n";
  rs << "  wire " << prefix << "_ack;\n";
  rs << "  assign " << prefix << "_en = "
     << JoinStatesWithSubState(callers, 0) << ";\n";
}

void SubModuleTask::BuildSubModuleTaskCallInsn(IInsn *insn, State *st) {
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

string SubModuleTask::SubModuleTaskControlPinPrefix(const IResource &res) {
  return "task_" + Util::Itoa(res.GetTable()->GetId())
    + "_" + Util::Itoa(res.GetId());
}

void SubModuleTask::BuildChildTaskWire(const TaskCallInfo &ti,
				       ostream &is) {
  for (IResource *caller : ti.tasks_) {
    ITable *tab = caller->GetTable();
    string caller_en;
    string caller_ack;
    string prefix = SubModuleTaskControlPinPrefix(*caller);
    caller_en = prefix + "_en";
    caller_ack = prefix + "_ack";
    is << ", .task_" << tab->GetId() << "_en(" << caller_en << ")";
    is << ", .task_" << tab->GetId() << "_ack(" << caller_ack << ")";
  }
}

void SubModuleTask::BuildPorts(const TaskCallInfo &ti, Ports *ports) {
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
