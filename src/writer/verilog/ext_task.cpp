#include "writer/verilog/ext_task.h"

#include "design/design_util.h"
#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"
#include "writer/verilog/insn_writer.h"
#include "writer/verilog/ports.h"
#include "writer/verilog/state.h"
#include "writer/verilog/table.h"
#include "writer/verilog/task.h"

namespace iroha {
namespace writer {
namespace verilog {

ExtTask::ExtTask(const IResource &res, const Table &table)
  : Resource(res, table) {
}

bool ExtTask::IsExtTask(const Table &table) {
  ITable *i_table = table.GetITable();
  IInsn *task_entry_insn = DesignUtil::FindTaskEntryInsn(i_table);
  if (task_entry_insn != nullptr) {
    auto rc = *(task_entry_insn->GetResource()->GetClass());
    return resource::IsExtTask(rc);
  }
  return false;
}

string ExtTask::TaskReadyPin(const Table &table) {
  ITable *i_table = table.GetITable();
  IInsn *task_entry_insn = DesignUtil::FindTaskEntryInsn(i_table);
  IResource *res = task_entry_insn->GetResource();
  return ReqValidPin(*res);
}

string ExtTask::ReqValidPin(const IResource &res) {
  return res.GetParams()->GetExtTaskName() + "_req_valid";
}

string ExtTask::ReqReadyPin(const IResource &res) {
  return res.GetParams()->GetExtTaskName() + "_req_ready";
}

string ExtTask::BusyPin(const IResource &res) {
  return res.GetParams()->GetExtTaskName() + "_busy";
}

string ExtTask::ResValidPin(const IResource &res) {
  return res.GetParams()->GetExtTaskName() + "_res_valid";
}

string ExtTask::ResReadyPin(const IResource &res) {
  return res.GetParams()->GetExtTaskName() + "_res_ready";
}

void ExtTask::BuildResource() {
  auto *klass = res_.GetClass();
  if (resource::IsExtTask(*klass)) {
    BuildExtTask();
  }
  if (resource::IsExtTaskDone(*klass)) {
    ostream &ss = tab_.StateOutputSectionStream();
    map<IState *, IInsn *> callers;
    CollectResourceCallers("", &callers);
    ss << "      " << ResValidPin(*(res_.GetParentResource()))
       << " <= " << JoinStatesWithSubState(callers, 0) << " & "
       << ResReadyPin(*(res_.GetParentResource()))
       << ";\n";
  }
}

void ExtTask::BuildInsn(IInsn *insn, State *st) {
  auto *klass = res_.GetClass();
  if (resource::IsExtTaskDone(*klass)) {
    ostream &os = st->StateBodySectionStream();
    string insn_st = InsnWriter::MultiCycleStateName(*(insn->GetResource()));
    static const char I[] = "          ";
    os << I << "if (" << insn_st << " == 0) begin\n"
       << I << "  if (" << ResReadyPin(*(res_.GetParentResource()))
       << ") begin\n"
       << I << "    " << insn_st << " <= 3;\n"
       << I << "  end\n"
       << I << "end\n";
  }
}

void ExtTask::BuildExtTask() {
  ostream &is = tab_.InitialValueSectionStream();
  is << "      " << ReqReadyPin(res_) << " <= 0;\n"
     << "      " << ResValidPin(res_) << " <= 0;\n"
     << "      " << BusyPin(res_) << " <= 0;\n";
  ostream &ss = tab_.StateOutputSectionStream();
  ss << "      " << ReqReadyPin(res_) << " <= "
     << ReqValidPin(res_) << " & ("
     << tab_.StateVariable() << " == `"
     << tab_.StateName(Task::kTaskEntryStateId) << ")"
     << ";\n";
  ss << "      " << BusyPin(res_) << " <= !("
     << tab_.StateVariable() << " == `"
     << tab_.StateName(Task::kTaskEntryStateId) << ");\n";
  BuildPorts();
}

void ExtTask::BuildPorts() {
  auto *params = res_.GetParams();
  string fn = params->GetExtTaskName();
  auto *ports = tab_.GetPorts();
  ports->AddPort(ReqValidPin(res_), Port::INPUT, 0);
  ports->AddPort(ReqReadyPin(res_), Port::OUTPUT, 0);
  ports->AddPort(BusyPin(res_), Port::OUTPUT, 0);
  ports->AddPort(ResValidPin(res_), Port::OUTPUT, 0);
  ports->AddPort(ResReadyPin(res_), Port::INPUT, 0);
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
