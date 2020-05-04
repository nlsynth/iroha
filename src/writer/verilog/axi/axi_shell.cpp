#include "writer/verilog/axi/axi_shell.h"

#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "writer/verilog/axi/axi_port.h"
#include "writer/verilog/axi/channel_generator.h"
#include "writer/verilog/port.h"
#include "writer/verilog/port_set.h"

namespace iroha {
namespace writer {
namespace verilog {
namespace axi {

AxiShell::AxiShell(IResource *res) : res_(res) {
  auto *klass = res->GetClass();
  is_master_ = resource::IsAxiMasterPort(*klass);
  PortConfig config = AxiPort::GetPortConfig(*res_);
  p_ = config.prefix;
}

string AxiShell::P(const string &p) {
  return p_ + p;
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

void AxiShell::WriteFSM(const PortSet *ports, bool reset_polarity,
			ostream &os) {
  if (!is_master_) {
    return;
  }
  PortConfig config = AxiPort::GetPortConfig(*res_);
  os << "\n"
     << "  // WIP: AXI shell FSM: " << config.prefix << "\n";
  const string &clk = ports->GetClk();
  const string &rst = ports->GetReset();
  os << "  reg [7:0] " << P("ARLEN_") << ";\n"
     << "  always @(posedge " << clk << ") begin\n"
     << "    if (" << (reset_polarity ? "" : "!") << rst << ") begin\n"
     << "      " << P("ARREADY") << " <= 0;\n"
     << "      " << P("RVALID") << " <= 0;\n"
     << "      " << P("AWREADY") << " <= 0;\n"
     << "      " << P("WREADY") << " <= 0;\n"
     << "      " << P("BVALID") << " <= 0;\n"
     << "    end else begin\n"
     << "      // READ\n"
     << "      " << P("ARREADY") << " <= " << P("ARVALID") << ";\n"
     << "      " << P("RVALID") << " <= " << P("RREADY") << ";\n"
     << "      " << P("RLAST") << " <= " << P("RREADY") << " && (" << P("ARLEN_") << " == 255);\n"
     << "      if (" << P("ARVALID") << ") begin\n"
     << "        " << P("ARLEN_") << " <= " << P("ARLEN") << ";\n"
     << "      end\n"
     << "      if (" << P("ARVALID") << ") begin\n"
     << "        " << P("ARLEN_") << " <= " << P("ARLEN") << ";\n"
     << "      end\n"
     << "      if (" << P("RVALID") << " && " << P("RREADY") << ") begin\n"
     << "        " << P("ARLEN_") << " <= " << P("ARLEN_") << " - 1;\n"
     << "      end\n"
     << "      // WRITE\n"
     << "      " << P("AWREADY") << " <= " << P("AWVALID") << ";\n"
     << "      " << P("WREADY") << " <= " << P("WVALID") << ";\n"
     << "      if (!" << P("BVALID") << " && " << P("WLAST") << " && " << P("WREADY") << ") begin\n"
     << "        " << P("BVALID") << " <= 1;\n"
     << "      end\n"
     << "      if (" << P("BVALID") << " && " << P("BREADY") << ") begin\n"
     << "        " << P("BVALID") << " <= 0;\n"
     << "      end\n"
     << "    end\n"
     << "  end\n";
}

}  // namespace axi
}  // namespace verilog
}  // namespace writer
}  // namespace iroha
