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

void ChannelGenerator::GenReadChannel(const PortConfig &cfg,
				      bool is_master, Module *module,
				      Ports *ports,
				      string *s) {
  AddPort(cfg, "ARADDR", cfg.axi_addr_width, false, is_master, -1, module, ports, s);
  AddPort(cfg, "ARVALID", 0, false, is_master, -1, module, ports, s);
  AddPort(cfg, "ARREADY", 0, true, is_master, -1, module, ports, s);
  AddPort(cfg, "ARLEN", 8, false, is_master, -1, module, ports, s);
  AddPort(cfg, "ARSIZE", 3, false, is_master, -1, module, ports, s);
  AddPort(cfg, "ARID", 1, false, is_master, 0, module, ports, s);
  AddPort(cfg, "ARBURST", 2, false, is_master, 1, module, ports, s);  // INCR
  AddPort(cfg, "ARLOCK", 0, false, is_master, 0, module, ports, s);
  AddPort(cfg, "ARCACHE", 4, false, is_master, 3, module, ports, s); // non cacheable. bufferable.
  AddPort(cfg, "ARPROT", 3, false, is_master, 1, module, ports, s); // unpriviledged, non secure.
  AddPort(cfg, "ARQOS", 4, false, is_master, 0, module, ports, s);
  AddPort(cfg, "ARUSER", 1, false, is_master, 0, module, ports, s);

  AddPort(cfg, "RVALID", 0, true, is_master, -1, module, ports, s);
  AddPort(cfg, "RDATA", cfg.data_width, true, is_master, -1, module, ports, s);
  AddPort(cfg, "RREADY", 0, false, is_master, -1, module, ports, s);
  AddPort(cfg, "RLAST", 0, true, is_master, -1, module, ports, s);
  AddPort(cfg, "RRESP", 2, true, is_master, 0, module, ports, s);
  AddPort(cfg, "RUSER", 1, true, is_master, 0, module, ports, s);
}

void ChannelGenerator::GenWriteChannel(const PortConfig &cfg,
				       bool is_master, Module *module,
				       Ports *ports,
				       string *s) {
  AddPort(cfg, "AWADDR", cfg.axi_addr_width, false, is_master, -1, module, ports, s);
  AddPort(cfg, "AWVALID", 0, false, is_master, -1, module, ports, s);
  AddPort(cfg, "AWREADY", 0, true, is_master, -1, module, ports, s);
  AddPort(cfg, "AWLEN", 8, false, is_master, -1, module, ports, s);
  AddPort(cfg, "AWSIZE", 3, false, is_master, -1, module, ports, s);
  AddPort(cfg, "AWID", 1, false, is_master, 0, module, ports, s);
  AddPort(cfg, "AWBURST", 2, false, is_master, 1, module, ports, s);  // INCR
  AddPort(cfg, "AWLOCK", 0, false, is_master, 0, module, ports, s);
  AddPort(cfg, "AWCACHE", 4, false, is_master, 3, module, ports, s); // non cacheable. bufferable.
  AddPort(cfg, "AWPROT", 3, false, is_master, 1, module, ports, s); // unpriviledged, non secure.
  AddPort(cfg, "AWQOS", 4, false, is_master, 0, module, ports, s);
  AddPort(cfg, "AWUSER", 1, false, is_master, 0, module, ports, s);

  AddPort(cfg, "WVALID", 0, false, is_master, -1, module, ports, s);
  AddPort(cfg, "WREADY", 0, true, is_master, -1, module, ports, s);
  AddPort(cfg, "WDATA", cfg.data_width, false, is_master, -1, module, ports, s);
  AddPort(cfg, "WLAST", 0, false, is_master, -1, module, ports, s);
  AddPort(cfg, "WSTRB", cfg.data_width / 8, false, is_master, kStrbMagic, module, ports, s);
  AddPort(cfg, "WUSER", 1, false, is_master, -1, module, ports, s);

  AddPort(cfg, "BVALID", 0, true, is_master, -1, module, ports, s);
  AddPort(cfg, "BREADY", 0, false, is_master, -1, module, ports, s);
  AddPort(cfg, "BRESP", 2, true, is_master, -1, module, ports, s);
}

void ChannelGenerator::AddPort(const PortConfig &cfg,
			       const string &name, int width, bool dir_s2m,
			       bool is_master,
			       int fixed_value,
			       Module *module, Ports *ports,
			       string *s) {
  // This method is used to add a port to either user's modules or
  // controller module.
  bool is_controller = (module == nullptr);
  Port::PortType t;
  bool is_input = false;
  if (is_master) {
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
    prefix = cfg.prefix;
  }
  Port *port = ports->AddPrefixedPort(prefix, name, t, width);
  if (t == Port::OUTPUT_WIRE && is_controller && is_fixed_output) {
    port->SetFixedValue(fixed_value);
  }
  // On the controller instantiation. Owner of the controller.
  if (!is_controller && s != nullptr) {
    string p = ", ." + name + "(" + cfg.prefix + name + ")";
    *s += p;
  }
  // Non controller.
  if (!is_controller) {
    Module *parent = module->GetParentModule();
    if (parent != nullptr) {
      ostream &os = parent->ChildModuleInstSectionStream(module);
      string p = ", ." + cfg.prefix + name + "(" + cfg.prefix + name + ")";
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
