#include "writer/verilog/self_shell.h"

#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "writer/verilog/axi/axi_shell.h"

namespace iroha {
namespace writer {
namespace verilog {

SelfShell::SelfShell(const IDesign *design) : design_(design) {
  for (IModule *mod : design->modules_) {
    ProcessModule(mod);
  }
}

void SelfShell::WriteWireDecl(ostream &os) {
  for (IResource *res : axi_) {
    axi::AxiShell shell(res);
    shell.WriteWireDecl(os);
  }
}

void SelfShell::WritePortConnection(ostream &os) {
  for (IResource *res : axi_) {
    axi::AxiShell shell(res);
    shell.WritePortConnection(os);
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
    }
  }
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
