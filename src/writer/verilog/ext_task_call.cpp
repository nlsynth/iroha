#include "writer/verilog/ext_task_call.h"

#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"
#include "writer/module_template.h"
#include "writer/verilog/ext_task.h"
#include "writer/verilog/insn_writer.h"
#include "writer/verilog/module.h"
#include "writer/verilog/ports.h"
#include "writer/verilog/state.h"
#include "writer/verilog/table.h"

namespace iroha {
namespace writer {
namespace verilog {

ExtTaskCall::ExtTaskCall(const IResource &res, const Table &table)
  : Resource(res, table) {
}

void ExtTaskCall::BuildResource() {
  auto *klass = res_.GetClass();
  if (resource::IsExtTaskCall(*klass)) {
    auto *params = res_.GetParams();
    string fn = params->GetExtTaskName();
    AddPortToTop(ExtTask::ReqValidPin(res_), true, 0);
    AddPortToTop(ExtTask::ReqReadyPin(res_), false, 0);
    for (int i = 0; i < res_.input_types_.size(); ++i) {
      AddPortToTop(ExtTask::ArgPin(res_, i), true,
		   res_.input_types_[i].GetWidth());
    }
    AddPortToTop(ExtTask::ResValidPin(res_), false, 0);
    AddPortToTop(ExtTask::ResReadyPin(res_), true, 0);
    // Finds corresponding ext-task-wait resource.
    const IResource *wait_res = nullptr;
    for (IResource *res : tab_.GetITable()->resources_) {
      if (res->GetParentResource() == &res_) {
	wait_res = &res_;
      }
    }
    ostream &rs = tmpl_->GetStream(kRegisterSection);
    for (int i = 0; i < wait_res->output_types_.size(); ++i) {
      AddPortToTop(ExtTask::DataPin(*wait_res, i), false,
		   wait_res->output_types_[i].GetWidth());
      rs << "  reg "
	 << Table::WidthSpec(wait_res->output_types_[i].GetWidth())
	 << " " << ResCaptureReg(i) << ";\n";
    }
  }
}

void ExtTaskCall::BuildInsn(IInsn *insn, State *st) {
  auto *klass = res_.GetClass();
  if (resource::IsExtTaskCall(*klass)) {
    ostream &os = st->StateBodySectionStream();
    string insn_st = InsnWriter::MultiCycleStateName(*(insn->GetResource()));
    static const char I[] = "          ";
    os << I << "if (" << insn_st << " == 0) begin\n"
       << I << "  " << ExtTask::ReqValidPin(res_)
       << " <= 1;\n"
       << I << "  " << insn_st << " <= 1;\n";
    for (int i = 0; i < insn->inputs_.size(); ++i) {
      os << I << "  " << ExtTask::ArgPin(res_, i) << " <= "
	 << InsnWriter::RegisterValue(*insn->inputs_[i], tab_.GetNames())
	 << ";\n";
    }
    os << I << "end\n";
    os << I << "if (" << insn_st << " == 1) begin\n"
       << I << "  if (" << ExtTask::ResValidPin(res_) << ") begin\n"
       << I << "    " << ExtTask::ReqValidPin(res_)
       << " <= 0;\n"
       << I << "    " << insn_st << " <= 3;\n"
       << I << "  end\n";
    os << I << "end\n";
  }
  if (resource::IsExtTaskWait(*klass)) {
    ostream &ws = tmpl_->GetStream(kInsnWireValueSection);
    ostream &rs = tmpl_->GetStream(kRegisterSection);
    for (int i = 0; i < insn->outputs_.size(); ++i) {
      rs << "  reg "
	 << Table::WidthSpec(res_.output_types_[i].GetWidth())
	 << " " << ResCaptureReg(i) << ";\n";
      ws << "  assign " << InsnWriter::InsnOutputWireName(*insn, i)
	 << " = " << ResCaptureReg(i) << ";\n";
    }
    ostream &os = st->StateBodySectionStream();
    string insn_st = InsnWriter::MultiCycleStateName(*(insn->GetResource()));
    static const char I[] = "          ";
    const IResource *call = res_.GetParentResource();
    os << I << "if (" << insn_st << " == 0) begin\n"
       << I << "  " << ExtTask::ResReadyPin(*call)
       << " <= 1;\n"
       << I << "  " << insn_st << " <= 1;\n"
       << I << "end\n"
       << I << "if (" << insn_st << " == 1) begin\n"
       << I << "  if (" << ExtTask::ResValidPin(*call) << ") begin\n"
       << I << "    " << ExtTask::ResReadyPin(*call)
       << " <= 0;\n"
       << I << "  " << insn_st << " <= 3;\n";
    for (int i = 0; i < insn->outputs_.size(); ++i) {
      os << I << "  " << ResCaptureReg(i) << " <= "
	 << InsnWriter::RegisterValue(*insn->outputs_[i], tab_.GetNames())
	 << ";\n";
    }
    os << I << "  end\n"
       << I << "end\n";
  }
}

string ExtTaskCall::ResCaptureReg(int nth) {
  return "ext_task_res_" + Util::Itoa(tab_.GetITable()->GetId()) + "_"
    + Util::Itoa(nth);
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
