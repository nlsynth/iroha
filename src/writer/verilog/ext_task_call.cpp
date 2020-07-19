#include "writer/verilog/ext_task_call.h"

#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"
#include "writer/module_template.h"
#include "writer/verilog/embedded_modules.h"
#include "writer/verilog/ext_task.h"
#include "writer/verilog/insn_writer.h"
#include "writer/verilog/module.h"
#include "writer/verilog/port.h"
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
  if (resource::IsExtTaskCall(*klass) || resource::IsExtFlowCall(*klass)) {
    BuildExtTaskCallResource();
  }
  // does nothing for resource::IsExtTaskWait().
}

void ExtTaskCall::BuildExtTaskCallResource() {
  auto *params = res_.GetParams();
  string fn = params->GetExtTaskName();
  bool use_handshake = UseHandShake();
  string connection;
  if (use_handshake) {
    AddPort(ExtTask::ReqValidPin(&res_), ExtTask::ReqValidPin(nullptr),
	    true, 0, &connection);
    AddPort(ExtTask::ReqReadyPin(&res_), ExtTask::ReqReadyPin(nullptr),
	    false, 0, &connection);
  }
  for (int i = 0; i < res_.input_types_.size(); ++i) {
    AddPort(ExtTask::ArgPin(&res_, i),
	    ExtTask::ArgPin(nullptr, i), true,
	    res_.input_types_[i].GetWidth(), &connection);
  }
  const IResource *wait_res = GetWaitResource();
  if (wait_res != nullptr) {
    if (use_handshake) {
      AddPort(ExtTask::ResValidPin(&res_), ExtTask::ResValidPin(nullptr),
	      false, 0, &connection);
      AddPort(ExtTask::ResReadyPin(&res_), ExtTask::ResReadyPin(nullptr),
	      true, 0, &connection);
    }
    ostream &rs = tab_.ResourceSectionStream();
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
    AddIO(&connection);
    BuildEmbeddedModule(connection);
  }
}

void ExtTaskCall::AddPort(const string &name, const string &wire_name,
			  bool is_output, int width,
			  string *connection) {
  ostream &rs = tab_.ResourceSectionStream();
  if (IsEmbedded()) {
    if (is_output) {
      rs << "  reg";
    } else {
      rs << "  wire";
    }
    rs << " " << Table::WidthSpec(width) << name << ";\n";
    *connection += ", ." + wire_name + "(" + name + ")";
  } else {
    AddPortToTop(name, is_output, false, width);
  }
}

void ExtTaskCall::BuildInsn(IInsn *insn, State *st) {
  auto *klass = res_.GetClass();
  if (resource::IsExtTaskCall(*klass)) {
    BuildTaskCallInsn(insn, st);
  }
  if (resource::IsExtTaskWait(*klass)) {
    BuildTaskWaitInsn(insn, st);
  }
  if (resource::IsExtFlowCall(*klass)) {
    BuildFlowCallInsn(insn, st);
  }
  if (resource::IsExtFlowResult(*klass)) {
    BuildFlowResultInsn(insn, st);
  }
}

void ExtTaskCall::BuildFlowCallInsn(IInsn *insn, State *st) {
  static const char I[] = "          ";
  ostream &os = st->StateBodySectionStream();
  for (int i = 0; i < insn->inputs_.size(); ++i) {
    os << I << ExtTask::ArgPin(&res_, i) << " <= "
       << InsnWriter::RegisterValue(*insn->inputs_[i], tab_.GetNames())
       << ";\n";
  }
}

void ExtTaskCall::BuildFlowResultInsn(IInsn *insn, State *st) {
  ostream &ws = tab_.InsnWireValueSectionStream();
  for (int i = 0; i < insn->outputs_.size(); ++i) {
    ws << "  assign " << InsnWriter::InsnOutputWireName(*insn, i)
       << " = " << ExtTask::DataPin(&res_, i) << ";\n";
  }
}

void ExtTaskCall::BuildTaskCallInsn(IInsn *insn, State *st) {
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

void ExtTaskCall::BuildTaskWaitInsn(IInsn *insn, State *st) {
  ostream &ws = tab_.InsnWireValueSectionStream();
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

void ExtTaskCall::AddIO(string *connection) {
  AddIOPorts(false, connection);
  AddIOPorts(true, connection);
}

void ExtTaskCall::AddIOPorts(bool is_output, string *connection) {
  auto *params = res_.GetParams();
  vector<string> ports = params->GetEmbeddedModuleIO(is_output);
  for (string &p : ports) {
    vector<string> s;
    Util::SplitStringUsing(p, ":", &s);
    string name = s[0];
    int w = 0;
    if (s.size() == 2) {
      w = Util::Atoi(s[1]);

    }
    AddPortToTop(name, is_output, true, w);
    *connection += ", ." + name + "(" + name + ")";
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

bool ExtTaskCall::UseHandShake() const {
  auto *klass = res_.GetClass();
  if (resource::IsExtFlowCall(*klass) || resource::IsExtFlowResult(*klass)) {
    return false;
  }
  return true;
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
