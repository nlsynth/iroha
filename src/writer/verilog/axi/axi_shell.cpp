#include "writer/verilog/axi/axi_shell.h"

#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "writer/verilog/axi/axi_port.h"
#include "writer/verilog/axi/channel_generator.h"

namespace iroha {
namespace writer {
namespace verilog {
namespace axi {

AxiShell::AxiShell(IResource *res) : res_(res) {
  auto *klass = res->GetClass();
  is_master_ = resource::IsAxiMasterPort(*klass);
}

void AxiShell::WriteWireDecl(ostream &os) {
  PortConfig config = AxiPort::GetPortConfig(*res_);
  os << "  // AXI shell wires: " << config.prefix << "\n";
  string s;
  ChannelGenerator ch(config, ChannelGenerator::SHELL_WIRE_DECL,
		      is_master_, nullptr, nullptr, &s);
  ch.GenerateChannel(true, true);
  os << s << "\n";
}

void AxiShell::WritePortConnection(ostream &os) {
  PortConfig config = AxiPort::GetPortConfig(*res_);
  os << "  /* AXI shell: " << config.prefix << " */";
  string s;
  ChannelGenerator ch(config, ChannelGenerator::SHELL_PORT_CONNECTION,
		      is_master_, nullptr, nullptr, &s);
  ch.GenerateChannel(true, true);
  os << s;
}

void AxiShell::WriteFSM(bool reset_polarity, ostream &os) {
  if (!is_master_) {
    return;
  }
  PortConfig config = AxiPort::GetPortConfig(*res_);
  os << "\n"
     << "  // WIP: AXI shell FSM: " << config.prefix << "\n"
     << "  // polarity=" << reset_polarity << "\n"; 
}

}  // namespace axi
}  // namespace verilog
}  // namespace writer
}  // namespace iroha
