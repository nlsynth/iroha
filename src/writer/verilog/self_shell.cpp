#include "writer/verilog/self_shell.h"

#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"
#include "writer/verilog/axi/axi_shell.h"
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
    }
  }
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
