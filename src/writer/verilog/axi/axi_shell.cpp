#include "writer/verilog/axi/axi_shell.h"

#include "writer/verilog/axi/axi_port.h"
#include "writer/verilog/axi/channel_generator.h"

namespace iroha {
namespace writer {
namespace verilog {
namespace axi {

AxiShell::AxiShell(IResource *res) : res_(res) {
  is_master_ = true;
}

void AxiShell::WriteWireDecl(ostream &os) {
  PortConfig config = AxiPort::GetPortConfig(*res_);
  os << "  // WIP AXI prefix " << config.prefix << "\n";
  string s;
  ChannelGenerator ch(config, ChannelGenerator::SHELL_WIRE_DECL,
		      is_master_, nullptr, nullptr, &s);
  ch.GenerateChannel(true, true);
  os << s;
}

void AxiShell::WritePortConnection(ostream &os) {
  PortConfig config = AxiPort::GetPortConfig(*res_);
  os << "  /* WIP AXI prefix " << config.prefix << " */";
  string s;
  ChannelGenerator ch(config, ChannelGenerator::SHELL_PORT_CONNECTION,
		      is_master_, nullptr, nullptr, &s);
  ch.GenerateChannel(true, true);
  os << s;
}

}  // namespace axi
}  // namespace verilog
}  // namespace writer
}  // namespace iroha
