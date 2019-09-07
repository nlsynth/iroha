#include "writer/verilog/self_shell.h"

#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"
#include "writer/verilog/axi/axi_shell.h"
#include "writer/verilog/ext_task.h"
#include "writer/verilog/table.h"

namespace iroha {
namespace writer {
namespace verilog {

SelfShell::SelfShell(const IDesign *design, const Ports *ports,
		     bool reset_polarity)
  : design_(design), ports_(ports), reset_polarity_(reset_polarity) {
  for (IModule *mod : design->modules_) {
    ProcessModule(mod);
  }
}

void SelfShell::WriteWireDecl(ostream &os) {
  for (IResource *res : axi_) {
    axi::AxiShell shell(res);
    shell.WriteWireDecl(os);
  }
  for (IResource *res : ext_input_) {
    auto *params = res->GetParams();
    string input_port;
    int width;
    params->GetExtInputPort(&input_port, &width);
    os << "  wire " << Table::WidthSpec(width) << input_port << ";\n";
    os << "  assign " << input_port << " = 0;\n";
  }
  for (IResource *res : ext_task_entry_) {
    string v = ExtTask::ReqValidPin(res);
    os << "  wire " << v << ";\n"
       << "  assign " << v << " = 0;\n";
  }
  for (IResource *res : ext_task_call_) {
    string req_ready = ExtTask::ReqReadyPin(res);
    os << "  wire " << req_ready << ";\n"
       << "  assign " << req_ready << " = 1;\n";
  }
  for (IResource *res : ext_task_wait_) {
    string res_valid = ExtTask::ResValidPin(res->GetParentResource());
    os << "  wire " << res_valid << ";\n"
       << "  assign " << res_valid << " = 1;\n";
    for (int i = 0; i < res->output_types_.size(); ++i) {
      string d = ExtTask::DataPin(res, i);
      os << "  wire "
	 << Table::WidthSpec(res->output_types_[i].GetWidth())
	 << d << ";\n"
	 << "  assign " << d << " = 0;\n";
    }
  }
}

void SelfShell::WritePortConnection(ostream &os) {
  for (IResource *res : axi_) {
    axi::AxiShell shell(res);
    shell.WritePortConnection(os);
  }
  for (IResource *res : ext_input_) {
    auto *params = res->GetParams();
    string input_port;
    int width;
    params->GetExtInputPort(&input_port, &width);
    os << ", ." << input_port << "(" << input_port << ")";
  }
  for (IResource *res : ext_task_entry_) {
    string v = ExtTask::ReqValidPin(res);
    os << ", ." << v << "(" << v << ")";
  }
  for (IResource *res : ext_task_call_) {
    string req_ready = ExtTask::ReqReadyPin(res);
    os << ", ." << req_ready << "(" << req_ready << ")";
  }
  for (IResource *res : ext_task_wait_) {
    string res_valid = ExtTask::ResValidPin(res->GetParentResource());
    os << ", ." << res_valid << "(" << res_valid << ")";
    for (int i = 0; i < res->output_types_.size(); ++i) {
      string d = ExtTask::DataPin(res, i);
      os << ", ." << d << "(" << d << ")";
    }
  }
}

void SelfShell::WriteShellFSM(ostream &os) {
  for (IResource *res : axi_) {
    axi::AxiShell shell(res);
    shell.WriteFSM(ports_, reset_polarity_, os);
  }
}

void SelfShell::ProcessModule(IModule *mod) {
  for (ITable *tab : mod->tables_) {
    for (IResource *res : tab->resources_) {
      auto *klass = res->GetClass();
      if (resource::IsAxiMasterPort(*klass) ||
	  resource::IsAxiSlavePort(*klass)) {
	axi_.push_back(res);
      }
      if (resource::IsExtInput(*klass)) {
	ext_input_.push_back(res);
      }
      if (resource::IsExtTask(*klass)) {
	ext_task_entry_.push_back(res);
      }
      if (resource::IsExtTaskCall(*klass)) {
	ext_task_call_.push_back(res);
      }
      if (resource::IsExtTaskWait(*klass)) {
	ext_task_wait_.push_back(res);
      }
    }
  }
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
