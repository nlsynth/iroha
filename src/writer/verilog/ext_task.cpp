#include "writer/verilog/ext_task.h"

#include "design/design_util.h"
#include "iroha/i_design.h"
#include "iroha/logging.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"
#include "writer/module_template.h"
#include "writer/verilog/insn_writer.h"
#include "writer/verilog/module.h"
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

string ExtTask::ArgPin(const IResource &res, int nth) {
  return
    res.GetParams()->GetExtTaskName() + "_req_" + Util::Itoa(nth);
}

string ExtTask::DataPin(const IResource &res, int nth) {
  IResource *parent = res.GetParentResource();
  return
    parent->GetParams()->GetExtTaskName() + "_res_" + Util::Itoa(nth);
}

void ExtTask::BuildResource() {
  auto *klass = res_.GetClass();
  if (resource::IsExtTask(*klass)) {
    BuildExtTask();
  }
}

void ExtTask::BuildInsn(IInsn *insn, State *st) {
  auto *klass = res_.GetClass();
  if (resource::IsExtTaskDone(*klass)) {
    ostream &os = st->StateBodySectionStream();
    string insn_st = InsnWriter::MultiCycleStateName(*(insn->GetResource()));
    static const char I[] = "          ";
    os << I << "if (" << insn_st << " == 0) begin\n"
       << I << "  " << ResValidPin(*(res_.GetParentResource())) << " <= 1;\n"
       << I << "  " << insn_st << " <= 1;\n";
    for (int i = 0; i < res_.input_types_.size(); ++i) {
      os << I << "  " << DataPin(res_, i) << " <= "
	 << InsnWriter::RegisterValue(*insn->inputs_[i], tab_.GetNames())
	 << ";\n";
    }
    os << I << "end\n";
    os << I << "if (" << insn_st << " == 1) begin\n"
       << I << "  if (" << ResReadyPin(*(res_.GetParentResource())) << ") begin\n"
       << I << "    " << ResValidPin(*(res_.GetParentResource())) << " <= 0;\n"
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
  ITable *i_table = tab_.GetITable();
  IInsn *task_entry_insn = DesignUtil::FindTaskEntryInsn(i_table);
  ostream &rs = tmpl_->GetStream(kRegisterSection);
  ostream &ws = tmpl_->GetStream(kInsnWireValueSection);
  ostream &ts = tab_.TaskEntrySectionStream();
  for (int i = 0; i < res_.output_types_.size(); ++i) {
    rs << "  reg "
       << Table::WidthSpec(res_.output_types_[i].GetWidth())
       << " " << ArgCaptureReg(i) << ";\n";
    ws << "  assign " << InsnWriter::InsnOutputWireName(*task_entry_insn, i)
       << " = " << ArgCaptureReg(i) << ";\n";
    ts << "            " << ArgCaptureReg(i) << " <= " << ArgPin(res_, i) << ";\n";
  }
  BuildPorts();
}

void ExtTask::BuildPorts() {
  auto *params = res_.GetParams();
  string fn = params->GetExtTaskName();
  AddPortToTop(ReqValidPin(res_), false, 0);
  AddPortToTop(ReqReadyPin(res_), true, 0);
  AddPortToTop(BusyPin(res_), true, 0);
  AddPortToTop(ResValidPin(res_), true, 0);
  AddPortToTop(ResReadyPin(res_), false, 0);
  for (int i = 0; i < res_.output_types_.size(); ++i) {
    AddPortToTop(ArgPin(res_, i), false,
		 res_.output_types_[i].GetWidth());
  }
  IResource *done_res =
    DesignUtil::FindOneResourceByClassName(tab_.GetITable(),
					   resource::kExtTaskDone);
  CHECK(done_res);
  for (int i = 0; i < done_res->input_types_.size(); ++i) {
    AddPortToTop(DataPin(*done_res, i), true,
		 done_res->input_types_[i].GetWidth());
  }
}

string ExtTask::ArgCaptureReg(int nth) {
  return "ext_task_arg_" + Util::Itoa(tab_.GetITable()->GetId()) + "_"
    + Util::Itoa(nth);
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
