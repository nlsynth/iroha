#include "writer/verilog/axi/channel_generator.h"

#include "iroha/i_design.h"
#include "writer/verilog/module.h"
#include "writer/verilog/ports.h"

namespace {
const int kStrbMagic = -7;
}  // namespace

namespace iroha {
namespace writer {
namespace verilog {
namespace axi {

ChannelGenerator::ChannelGenerator(const PortConfig &cfg,
				   bool is_master)
  : cfg_(cfg), is_master_(is_master) {
}

void ChannelGenerator::GenReadChannel(Module *module, Ports *ports,
				      string *s) {
  AddPort("ARADDR", cfg_.axi_addr_width, false, -1, module, ports, s);
  AddPort("ARVALID", 0, false, -1, module, ports, s);
  AddPort("ARREADY", 0, true, -1, module, ports, s);
  AddPort("ARLEN", 8, false, -1, module, ports, s);
  AddPort("ARSIZE", 3, false, -1, module, ports, s);
  AddPort("ARID", 1, false, 0, module, ports, s);
  AddPort("ARBURST", 2, false, 1, module, ports, s);  // INCR
  AddPort("ARLOCK", 0, false, 0, module, ports, s);
  AddPort("ARCACHE", 4, false, 3, module, ports, s); // non cacheable. bufferable.
  AddPort("ARPROT", 3, false, 1, module, ports, s); // unpriviledged, non secure.
  AddPort("ARQOS", 4, false, 0, module, ports, s);
  AddPort("ARUSER", 1, false, 0, module, ports, s);

  AddPort("RVALID", 0, true, -1, module, ports, s);
  AddPort("RDATA", cfg_.data_width, true, -1, module, ports, s);
  AddPort("RREADY", 0, false, -1, module, ports, s);
  AddPort("RLAST", 0, true, -1, module, ports, s);
  AddPort("RRESP", 2, true, 0, module, ports, s);
  AddPort("RUSER", 1, true, 0, module, ports, s);
}

void ChannelGenerator::GenWriteChannel(Module *module, Ports *ports,
				       string *s) {
  AddPort("AWADDR", cfg_.axi_addr_width, false, -1, module, ports, s);
  AddPort("AWVALID", 0, false, -1, module, ports, s);
  AddPort("AWREADY", 0, true, -1, module, ports, s);
  AddPort("AWLEN", 8, false, -1, module, ports, s);
  AddPort("AWSIZE", 3, false, -1, module, ports, s);
  AddPort("AWID", 1, false, 0, module, ports, s);
  AddPort("AWBURST", 2, false, 1, module, ports, s);  // INCR
  AddPort("AWLOCK", 0, false, 0, module, ports, s);
  AddPort("AWCACHE", 4, false, 3, module, ports, s); // non cacheable. bufferable.
  AddPort("AWPROT", 3, false, 1, module, ports, s); // unpriviledged, non secure.
  AddPort("AWQOS", 4, false, 0, module, ports, s);
  AddPort("AWUSER", 1, false, 0, module, ports, s);

  AddPort("WVALID", 0, false, -1, module, ports, s);
  AddPort("WREADY", 0, true, -1, module, ports, s);
  AddPort("WDATA", cfg_.data_width, false, -1, module, ports, s);
  AddPort("WLAST", 0, false, -1, module, ports, s);
  AddPort("WSTRB", cfg_.data_width / 8, false, kStrbMagic, module, ports, s);
  AddPort("WUSER", 1, false, -1, module, ports, s);

  AddPort("BVALID", 0, true, -1, module, ports, s);
  AddPort("BREADY", 0, false, -1, module, ports, s);
  AddPort("BRESP", 2, true, -1, module, ports, s);
}

void ChannelGenerator::AddPort(const string &name, int width, bool dir_s2m,
			       int fixed_value,
			       Module *module, Ports *ports,
			       string *s) {
  // This method is used to add a port to either user's modules or
  // controller module.
  bool is_controller = (module == nullptr);
  Port::PortType t;
  bool is_input = false;
  if (is_master_) {
    is_input = dir_s2m;
  } else {
    is_input = !dir_s2m;
  }
  bool is_fixed_output = (fixed_value >= 0) && !is_input;
  if (is_input) {
    t = Port::INPUT;
  } else {
    if (is_controller && !is_fixed_output) {
      t = Port::OUTPUT;
    } else {
      t = Port::OUTPUT_WIRE;
    }
  }
  string prefix;
  if (!is_controller) {
    prefix = cfg_.prefix;
  }
  Port *port = ports->AddPrefixedPort(prefix, name, t, width);
  if (t == Port::OUTPUT_WIRE && is_controller && is_fixed_output) {
    port->SetFixedValue(fixed_value);
  }
  // On the controller instantiation. Owner of the controller.
  if (!is_controller && s != nullptr) {
    string p = ", ." + name + "(" + cfg_.prefix + name + ")";
    *s += p;
  }
  // Non controller.
  if (!is_controller) {
    Module *parent = module->GetParentModule();
    if (parent != nullptr) {
      ostream &os = parent->ChildModuleInstSectionStream(module);
      string p = ", ." + cfg_.prefix + name + "(" + cfg_.prefix + name + ")";
      os << p;
    }
  }
  // Controller.
  if (is_controller && !is_input) {
    if (!is_fixed_output) {
      *s += "      " + name + " <= ";
      if (fixed_value == kStrbMagic) {
	// NOTE: Kludge to handle 1024 bit data bus width.
	// WSTRB can be up to 128 bits, but I don't want to exapnd
	// fixed_value width for now...
	*s += Util::Itoa(width) + "'b";
	for (int i = 0; i < width; ++i) {
	  *s += "1";
	}
      } else {
	*s += "0";
      }
      *s += ";\n";
    }
  }
}

}  // namespace axi
}  // namespace verilog
}  // namespace writer
}  // namespace iroha
