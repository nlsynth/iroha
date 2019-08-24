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
  AddPort("ARADDR", cfg_.axi_addr_width, false, -1);
  AddPort("ARVALID", 0, false, -1);
  AddPort("ARREADY", 0, true, -1);
  AddPort("ARLEN", 8, false, -1);
  AddPort("ARSIZE", 3, false, -1);
  AddPort("ARID", 1, false, 0);
  AddPort("ARBURST", 2, false, 1);  // INCR
  AddPort("ARLOCK", 0, false, 0);
  AddPort("ARCACHE", 4, false, 3); // non cacheable. bufferable.
  AddPort("ARPROT", 3, false, 1); // unpriviledged, non secure.
  AddPort("ARQOS", 4, false, 0);
  AddPort("ARUSER", 1, false, 0);

  AddPort("RVALID", 0, true, -1);
  AddPort("RDATA", cfg_.data_width, true, -1);
  AddPort("RREADY", 0, false, -1);
  AddPort("RLAST", 0, true, -1);
  AddPort("RRESP", 2, true, 0);
  AddPort("RUSER", 1, true, 0);
}

void ChannelGenerator::GenWriteChannel() {
  AddPort("AWADDR", cfg_.axi_addr_width, false, -1);
  AddPort("AWVALID", 0, false, -1);
  AddPort("AWREADY", 0, true, -1);
  AddPort("AWLEN", 8, false, -1);
  AddPort("AWSIZE", 3, false, -1);
  AddPort("AWID", 1, false, 0);
  AddPort("AWBURST", 2, false, 1);  // INCR
  AddPort("AWLOCK", 0, false, 0);
  AddPort("AWCACHE", 4, false, 3); // non cacheable. bufferable.
  AddPort("AWPROT", 3, false, 1); // unpriviledged, non secure.
  AddPort("AWQOS", 4, false, 0);
  AddPort("AWUSER", 1, false, 0);

  AddPort("WVALID", 0, false, -1);
  AddPort("WREADY", 0, true, -1);
  AddPort("WDATA", cfg_.data_width, false, -1);
  AddPort("WLAST", 0, false, -1);
  AddPort("WSTRB", cfg_.data_width / 8, false, kStrbMagic);
  AddPort("WUSER", 1, false, -1);

  AddPort("BVALID", 0, true, -1);
  AddPort("BREADY", 0, false, -1);
  AddPort("BRESP", 2, true, -1);
}

void ChannelGenerator::AddPort(const string &name, int width, bool dir_s2m,
			       int fixed_value) {
  // This method is used to add a port to either user's modules or
  // controller module.
  bool is_input = false;
  if (is_master_) {
    is_input = dir_s2m;
  } else {
    is_input = !dir_s2m;
  }
  bool is_fixed_output = (fixed_value >= 0) && !is_input;
  DoAddPort(name, width, is_input, is_fixed_output, fixed_value);
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

}  // namespace axi
}  // namespace verilog
}  // namespace writer
}  // namespace iroha
