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
  : SiblingTask(res, table) {
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
    string pin = SubModuleTaskCallerPinPrefix(res_);
    return pin + "_ack";
  }
  return "";
}

void SubModuleTask::BuildSubModuleTask() {
  string ack = PortNamePrefix(*tab_.GetITable()) + "ack";
  string en = PortNamePrefix(*tab_.GetITable()) + "en";
  ostream &fs = tab_.StateOutputSectionStream();
  fs << "      " << ack <<
    " <= (" << tab_.StateVariable() << " == `"
     << tab_.StateName(SiblingTask::kTaskEntryStateId) << ") && " << en
     << ";\n";
}

void SubModuleTask::BuildSubModuleTaskCall() {
  map<IState *, IInsn *> callers;
  CollectResourceCallers("", &callers);

  ostream &rs = tmpl_->GetStream(kResourceSection);
  string prefix = SubModuleTaskCallerPinPrefix(res_);
  rs << "  wire " << prefix << "_en;\n";
  rs << "  wire " << prefix << "_ack;\n";
  rs << "  assign " << prefix << "_en = "
     << JoinStatesWithSubState(callers, 0) << ";\n";
}

void SubModuleTask::BuildSubModuleTaskCallInsn(IInsn *insn, State *st) {
  ostream &os = st->StateBodySectionStream();
  static const char I[] = "          ";
  string st_name = InsnWriter::MultiCycleStateName(*(insn->GetResource()));
  IResource *res = insn->GetResource();
  string pin = SubModuleTaskCallerPinPrefix(*res);
  os << I << "if (" << st_name << " == 0) begin\n"
     << I << "  if (" << pin << "_ack) begin\n"
     << I << "    " << st_name << " <= 3;\n"
     << I << "  end else begin\n"
     << I << "  end\n"
     << I << "end\n";
}

string SubModuleTask::SubModuleTaskCallerPinPrefix(const IResource &res) {
  return "task_" + Util::Itoa(res.GetTable()->GetId())
    + "_" + Util::Itoa(res.GetId());
}

void SubModuleTask::BuildChildTaskWire(const TaskCallInfo &ti,
				       ostream &is) {
  for (IResource *caller : ti.tasks_) {
    string caller_prefix = SubModuleTaskCallerPinPrefix(*caller);
    string caller_en = caller_prefix + "_en";
    string caller_ack = caller_prefix + "_ack";
    ITable *callee_tab = caller->GetCalleeTable();
    string callee_prefix = PortNamePrefix(*callee_tab);
    is << ", ." << callee_prefix << "en(" << caller_en << ")";
    is << ", ." << callee_prefix << "ack(" << caller_ack << ")";
  }
}

void SubModuleTask::BuildPorts(const TaskCallInfo &ti, Ports *ports) {
  for (IResource *caller : ti.tasks_) {
    ITable *callee_tab = caller->GetCalleeTable();
    string prefix = PortNamePrefix(*callee_tab);
    string en = prefix + "en";
    ports->AddPort(en, Port::INPUT, 0);
    string ack = prefix + "ack";
    ports->AddPort(ack, Port::OUTPUT, 0);
  }
}

string SubModuleTask::PortNamePrefix(const ITable &callee_tab) {
  const IModule *mod = callee_tab.GetModule();
  return "task_" + mod->GetName() + "_" + Util::Itoa(callee_tab.GetId()) + "_";
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
