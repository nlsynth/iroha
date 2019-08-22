#include "writer/verilog/axi/axi_shell.h"

#include "writer/verilog/axi/axi_port.h"

namespace iroha {
namespace writer {
namespace verilog {
namespace axi {

AxiShell::AxiShell(IResource *res) : res_(res) {
}

void AxiShell::WriteWireDecl(ostream &os) {
  // WIP
  PortConfig config = AxiPort::GetPortConfig(*res_);
  os << "  // AXI prefix " << config.prefix << "\n";
}

void AxiShell::WritePortConnection(ostream &os) {
  // WIP
  PortConfig config = AxiPort::GetPortConfig(*res_);
  os << "  /* AXI prefix " << config.prefix << " */";
}

}  // namespace axi
}  // namespace verilog
}  // namespace writer
}  // namespace iroha
