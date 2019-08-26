#include "writer/verilog/axi/channel_generator.h"

#include "iroha/i_design.h"
#include "writer/verilog/module.h"
#include "writer/verilog/ports.h"
#include "writer/verilog/table.h"

namespace {
const int kStrbMagic = -7;
}  // namespace

namespace iroha {
namespace writer {
namespace verilog {
namespace axi {

ChannelGenerator::ChannelGenerator(const PortConfig &cfg,
				   enum OutputType type,
				   bool is_master, Module *module,
				   Ports *ports, string *s)
  : cfg_(cfg), type_(type), is_master_(is_master), module_(module),
    ports_(ports), s_(s) {
}

void ChannelGenerator::GenerateChannel(bool r, bool w) {
  if (r) {
    GenReadChannel();
  }
  if (w) {
    GenWriteChannel();
  }
}

void ChannelGenerator::GenReadChannel() {
  AddPort("ARADDR", cfg_.axi_addr_width, false, -1, false);
  AddPort("ARVALID", 0, false, -1, false);
  AddPort("ARREADY", 0, true, -1, false);
  AddPort("ARLEN", 8, false, -1, false);
  AddPort("ARSIZE", 3, false, -1, false);
  AddPort("ARID", 1, false, 0, false);
  AddPort("ARBURST", 2, false, 1, false);  // INCR
  AddPort("ARLOCK", 0, false, 0, false);
  AddPort("ARCACHE", 4, false, 3, false); // non cacheable. bufferable.
  AddPort("ARPROT", 3, false, 1, false); // unpriviledged, non secure.
  AddPort("ARQOS", 4, false, 0, false);
  AddPort("ARUSER", 1, false, 0, false);

  AddPort("RVALID", 0, true, -1, false);
  AddPort("RDATA", cfg_.data_width, true, -1, false);
  AddPort("RREADY", 0, false, -1, false);
  AddPort("RLAST", 0, true, -1, false);
  AddPort("RRESP", 2, true, 0, false);
  AddPort("RUSER", 1, true, 0, false);
}

void ChannelGenerator::GenWriteChannel() {
  AddPort("AWADDR", cfg_.axi_addr_width, false, -1, false);
  AddPort("AWVALID", 0, false, -1, false);
  AddPort("AWREADY", 0, true, -1, false);
  AddPort("AWLEN", 8, false, -1, false);
  AddPort("AWSIZE", 3, false, -1, false);
  AddPort("AWID", 1, false, 0, false);
  AddPort("AWBURST", 2, false, 1, false);  // INCR
  AddPort("AWLOCK", 0, false, 0, false);
  AddPort("AWCACHE", 4, false, 3, false); // non cacheable. bufferable.
  AddPort("AWPROT", 3, false, 1, false); // unpriviledged, non secure.
  AddPort("AWQOS", 4, false, 0, false);
  AddPort("AWUSER", 1, false, 0, false);

  AddPort("WVALID", 0, false, -1, false);
  AddPort("WREADY", 0, true, -1, false);
  AddPort("WDATA", cfg_.data_width, false, -1, false);
  AddPort("WLAST", 0, false, -1, false);
  AddPort("WSTRB", cfg_.data_width / 8, false, kStrbMagic, false);
  AddPort("WUSER", 1, false, -1, false);

  AddPort("BVALID", 0, true, -1, false);
  AddPort("BREADY", 0, false, -1, false);
  AddPort("BRESP", 2, true, -1, false);
}

void ChannelGenerator::AddPort(const string &name, int width, bool dir_s2m,
			       int fixed_value, bool drive_from_shell) {
  // This method is used to add a port to either user's modules or
  // controller module.
  bool is_input = false;
  if (is_master_) {
    is_input = dir_s2m;
  } else {
    is_input = !dir_s2m;
  }
  bool is_fixed_output = (fixed_value >= 0) && !is_input;
  if (type_ == CONTROLLER_PORTS_AND_REG_INITIALS ||
      type_ == PORTS_TO_EXT_AND_CONNECTIONS) {
    DoAddPort(name, width, is_input, is_fixed_output, fixed_value);
  }
  // On the controller instantiation. Owner of the controller.
  if (type_ == PORTS_TO_EXT_AND_CONNECTIONS) {
    string p = ", ." + name + "(" + cfg_.prefix + name + ")";
    *s_ += p;
  }
  // Non controller.
  if (type_ == PORTS_TO_EXT_AND_CONNECTIONS) {
    Module *parent = module_->GetParentModule();
    if (parent != nullptr) {
      ostream &os = parent->ChildModuleInstSectionStream(module_);
      string p = ", ." + cfg_.prefix + name + "(" + cfg_.prefix + name + ")";
      os << p;
    }
  }
  // Controller.
  // Initial value on reset.
  if (type_ == CONTROLLER_PORTS_AND_REG_INITIALS) {
    if (!is_input && !is_fixed_output) {
      MayAddInitialRegValue(name, width, fixed_value);
    }
  }
  if (type_ == SHELL_WIRE_DECL ||
      type_ == SHELL_PORT_CONNECTION) {
    WriteShellConnection(name, width, is_input, drive_from_shell);
  }
}

void ChannelGenerator::DoAddPort(const string &name, int width, bool is_input,
				 bool is_fixed_output,
				 int fixed_value) {
  Port::PortType t;
  if (is_input) {
    t = Port::INPUT;
  } else {
    if (type_ == CONTROLLER_PORTS_AND_REG_INITIALS && !is_fixed_output) {
      t = Port::OUTPUT;
    } else {
      t = Port::OUTPUT_WIRE;
    }
  }
  string prefix;
  if (type_ == PORTS_TO_EXT_AND_CONNECTIONS) {
    prefix = cfg_.prefix;
  }
  Port *port = ports_->AddPrefixedPort(prefix, name, t, width);
  if (type_ == CONTROLLER_PORTS_AND_REG_INITIALS &&
      t == Port::OUTPUT_WIRE && is_fixed_output) {
    port->SetFixedValue(fixed_value);
  }
}

void ChannelGenerator::MayAddInitialRegValue(const string &name, int width,
					     int fixed_value) {
  *s_ += "      " + name + " <= ";
  if (fixed_value == kStrbMagic) {
    // NOTE: Kludge to handle 1024 bit data bus width.
    // WSTRB can be up to 128 bits, but I don't want to use 128 bits
    // integer (e.g. uint128_t) to represent fixed_value width for now...
    *s_ += Util::Itoa(width) + "'b";
    for (int i = 0; i < width; ++i) {
      *s_ += "1";
    }
  } else {
    *s_ += "0";
  }
  *s_ += ";\n";
}

void ChannelGenerator::WriteShellConnection(const string &name, int width,
					    bool is_input,
					    bool drive_from_shell) {
  if (type_ == SHELL_WIRE_DECL) {
    string storage = "wire";
    if (is_input && drive_from_shell) {
      storage = "reg";
    }
    string n = cfg_.prefix + name;
    *s_ += "  " + storage + " " + Table::WidthSpec(width) + n + ";\n";
    if (is_input && !drive_from_shell) {
      *s_ += "  assign " + n + " = 0;\n";
    }
  }
  if (type_ == SHELL_PORT_CONNECTION) {
    string n = cfg_.prefix + name;
    *s_ += ", ." + n + "(" + n + ")";
  }
}

}  // namespace axi
}  // namespace verilog
}  // namespace writer
}  // namespace iroha
