#include "writer/verilog/ext_task_call.h"

#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"
#include "writer/module_template.h"
#include "writer/verilog/embed.h"
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
  // Embedded resource is also allowed for compatibility.
  if (resource::IsExtTaskCall(*klass) || resource::IsEmbedded(*klass)) {
    BuildExtTaskCallResource();
  }
  // does nothing for resource::IsExtTaskWait().
}

void ExtTaskCall::BuildExtTaskCallResource() {
  auto *params = res_.GetParams();
  string fn = params->GetExtTaskName();
  string connection;
  AddPort(ExtTask::ReqValidPin(&res_), ExtTask::ReqValidPin(nullptr),
	  true, 0, &connection);
  AddPort(ExtTask::ReqReadyPin(&res_), ExtTask::ReqReadyPin(nullptr),
	  false, 0, &connection);
  for (int i = 0; i < res_.input_types_.size(); ++i) {
    AddPort(ExtTask::ArgPin(&res_, i),
	    ExtTask::ArgPin(nullptr, i), true,
	    res_.input_types_[i].GetWidth(), &connection);
  }
  const IResource *wait_res = GetWaitResource();
  if (wait_res != nullptr) {
    AddPort(ExtTask::ResValidPin(&res_), ExtTask::ResValidPin(nullptr),
	    false, 0, &connection);
    AddPort(ExtTask::ResReadyPin(&res_), ExtTask::ResReadyPin(nullptr),
	    true, 0, &connection);
    ostream &rs = tmpl_->GetStream(kRegisterSection);
    for (int i = 0; i < wait_res->output_types_.size(); ++i) {
      AddPort(ExtTask::DataPin(wait_res, i),
	      ExtTask::DataPin(nullptr, i), false,
	      wait_res->output_types_[i].GetWidth(), &connection);
      rs << "  reg "
	 << Table::WidthSpec(wait_res->output_types_[i].GetWidth())
	 << " " << ResCaptureReg(i) << ";\n";
    }
  }
  if (IsEmbedded()) {
    BuildEmbeddedModule(connection);
  }
}

void ExtTaskCall::AddPort(const string &name, const string &wire_name,
			  bool is_output, int width,
			  string *connection) {
  ostream &rs = tmpl_->GetStream(kRegisterSection);
  if (IsEmbedded()) {
    if (is_output) {
      rs << "  reg";
    } else {
      rs << "  wire";
    }
    rs << " " << Table::WidthSpec(width) << name << ";\n";
    *connection += ", ." + wire_name + "(" + name + ")";
  } else {
    AddPortToTop(name, is_output, width);
  }
}

void ExtTaskCall::BuildEmbeddedModule(const string &connection) {
  auto *params = res_.GetParams();
  tab_.GetEmbeddedModules()->RequestModule(*params);

  auto *ports = tab_.GetPorts();
  ostream &is = tmpl_->GetStream(kEmbeddedInstanceSection);
  string name = params->GetEmbeddedModuleName();
  is << "  // " << name << "\n"
     << "  "  << name << " inst_" << tab_.GetITable()->GetId() << "_" << name
     << "(";
  is << "." << params->GetEmbeddedModuleClk() << "(" << ports->GetClk() << "), "
     << "." << params->GetEmbeddedModuleReset() << "(" << ports->GetReset() << ")";
  is << connection;
  is << ");\n";
}

void ExtTaskCall::BuildInsn(IInsn *insn, State *st) {
  auto *klass = res_.GetClass();
  if (resource::IsExtTaskCall(*klass) || resource::IsEmbedded(*klass)) {
    ostream &os = st->StateBodySectionStream();
    string insn_st = InsnWriter::MultiCycleStateName(*(insn->GetResource()));
    static const char I[] = "          ";
    os << I << "if (" << insn_st << " == 0) begin\n"
       << I << "  " << ExtTask::ReqValidPin(&res_)
       << " <= 1;\n"
       << I << "  " << insn_st << " <= 1;\n";
    for (int i = 0; i < insn->inputs_.size(); ++i) {
      os << I << "  " << ExtTask::ArgPin(&res_, i) << " <= "
	 << InsnWriter::RegisterValue(*insn->inputs_[i], tab_.GetNames())
	 << ";\n";
    }
    os << I << "end\n";
    os << I << "if (" << insn_st << " == 1) begin\n"
       << I << "  if (" << ExtTask::ReqReadyPin(&res_) << ") begin\n"
       << I << "    " << ExtTask::ReqValidPin(&res_)
       << " <= 0;\n"
       << I << "    " << insn_st << " <= 3;\n"
       << I << "  end\n";
    os << I << "end\n";
  }
  if (resource::IsExtTaskWait(*klass)) {
    ostream &ws = tmpl_->GetStream(kInsnWireValueSection);
    for (int i = 0; i < insn->outputs_.size(); ++i) {
      ws << "  assign " << InsnWriter::InsnOutputWireName(*insn, i)
	 << " = " << ResCaptureReg(i) << ";\n";
    }
    ostream &os = st->StateBodySectionStream();
    string insn_st = InsnWriter::MultiCycleStateName(*(insn->GetResource()));
    static const char I[] = "          ";
    const IResource *call = res_.GetParentResource();
    os << I << "if (" << insn_st << " == 0) begin\n"
       << I << "  " << ExtTask::ResReadyPin(call)
       << " <= 1;\n"
       << I << "  " << insn_st << " <= 1;\n"
       << I << "end\n"
       << I << "if (" << insn_st << " == 1) begin\n"
       << I << "  if (" << ExtTask::ResValidPin(call) << ") begin\n"
       << I << "    " << ExtTask::ResReadyPin(call)
       << " <= 0;\n"
       << I << "  " << insn_st << " <= 3;\n";
    for (int i = 0; i < insn->outputs_.size(); ++i) {
      os << I << "  " << ResCaptureReg(i) << " <= "
	 << ExtTask::DataPin(&res_, i)
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

bool ExtTaskCall::IsEmbedded() const {
  auto *params = res_.GetParams();
  return !(params->GetEmbeddedModuleName().empty());
}

const IResource *ExtTaskCall::GetWaitResource() const {
  const IResource *wait_res = nullptr;
  for (IResource *res : tab_.GetITable()->resources_) {
    if (res->GetParentResource() == &res_) {
      wait_res = res;
    }
  }
  return wait_res;
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
