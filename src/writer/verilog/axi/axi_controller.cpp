#include "writer/verilog/axi/axi_controller.h"

#include "iroha/i_design.h"
#include "writer/verilog/axi/axi_port.h"
#include "writer/verilog/module.h"
#include "writer/verilog/ports.h"

namespace iroha {
namespace writer {
namespace verilog {
namespace axi {

AxiController::AxiController(const IResource &res,
			     bool reset_polarity)
  : res_(res), reset_polarity_(reset_polarity) {
  ports_.reset(new Ports);
  cfg_ = AxiPort::GetPortConfig(res);
  const IResource *mem_res = res_.GetParentResource();
  IArray *array = mem_res->GetArray();
  sram_addr_width_ = array->GetAddressWidth();
}

AxiController::~AxiController() {
}

string AxiController::ResetName(bool polarity) {
  if (polarity) {
    return "rst";
  } else {
    return "rst_n";
  }
}

void AxiController::GenReadChannel(const PortConfig &cfg,
				   bool is_master, Module *module,
				   Ports *ports,
				   string *s) {
  // TODO: More ports.
  AddPort(cfg, "ARADDR", cfg.addr_width, false, is_master, module, ports, s);
  AddPort(cfg, "ARVALID", 0, false, is_master, module, ports, s);
  AddPort(cfg, "ARREADY", 0, true, is_master, module, ports, s);
  AddPort(cfg, "ARLEN", 8, false, is_master, module, ports, s);
  AddPort(cfg, "ARSIZE", 3, false, is_master, module, ports, s);

  AddPort(cfg, "RVALID", 0, true, is_master, module, ports, s);
  AddPort(cfg, "RDATA", cfg.data_width, true, is_master, module, ports, s);
  AddPort(cfg, "RREADY", 0, false, is_master, module, ports, s);
  AddPort(cfg, "RLAST", 0, true, is_master, module, ports, s);
}

void AxiController::GenWriteChannel(const PortConfig &cfg,
				    bool is_master, Module *module,
				    Ports *ports,
				    string *s) {
  AddPort(cfg, "AWADDR", cfg.addr_width, false, is_master, module, ports, s);
  AddPort(cfg, "AWVALID", 0, false, is_master, module, ports, s);
  AddPort(cfg, "AWREADY", 0, true, is_master, module, ports, s);
  AddPort(cfg, "AWLEN", 8, false, is_master, module, ports, s);
  AddPort(cfg, "AWSIZE", 3, false, is_master, module, ports, s);

  AddPort(cfg, "WVALID", 0, false, is_master, module, ports, s);
  AddPort(cfg, "WREADY", 0, true, is_master, module, ports, s);
  AddPort(cfg, "WDATA", cfg.data_width, false, is_master, module, ports, s);
  AddPort(cfg, "WLAST", 0, false, is_master, module, ports, s);

  AddPort(cfg, "BVALID", 0, true, is_master, module, ports, s);
  AddPort(cfg, "BREADY", 0, false, is_master, module, ports, s);
  AddPort(cfg, "BRESP", 2, true, is_master, module, ports, s);
}

void AxiController::AddPort(const PortConfig &cfg,
			    const string &name, int width, bool dir_s2m,
			    bool is_master,
			    Module *module, Ports *ports,
			    string *s) {
  Port::PortType t;
  bool is_input = false;
  if (is_master) {
    is_input = dir_s2m;
  } else {
    is_input = !dir_s2m;
  }
  if (is_input) {
    t = Port::INPUT;
  } else {
    if (module == nullptr) {
      t = Port::OUTPUT;
    } else {
      t = Port::OUTPUT_WIRE;
    }
  }
  string ext_port_name = name;
  if (module != nullptr) {
    ext_port_name = cfg.prefix + name;
  }
  ports->AddPort(ext_port_name, t, width);
  // On the controller instantiation. Owner of the controller.
  if (module != nullptr && s != nullptr) {
    string p = ", ." + name + "(" + cfg.prefix + name + ")";
    *s += p;
  }
  // Non controller.
  if (module != nullptr) {
    Module *parent = module->GetParentModule();
    if (parent != nullptr) {
      ostream &os = parent->ChildModuleInstSectionStream(module);
      string p = ", ." + cfg.prefix + name + "(" + cfg.prefix + name + ")";
      os << p;
    }
  }
  // Controller.
  if (module == nullptr) {
    if (!is_input) {
      *s += "      " + name + " <= 0;\n";
    }
  }
}

void AxiController::AddSramPorts() {
  ports_->AddPort("clk", Port::INPUT_CLK, 0);
  ports_->AddPort(ResetName(reset_polarity_), Port::INPUT_RESET, 0);
  ports_->AddPort("sram_addr", Port::OUTPUT, sram_addr_width_);
  ports_->AddPort("sram_wdata", Port::OUTPUT, cfg_.data_width);
  ports_->AddPort("sram_wen", Port::OUTPUT, 0);
  ports_->AddPort("sram_rdata", Port::INPUT, cfg_.data_width);
  ports_->AddPort("sram_EXCLUSIVE", Port::INPUT, 0);
  ports_->AddPort("sram_req", Port::OUTPUT, 0);
  ports_->AddPort("sram_ack", Port::INPUT, 0);
}

}  // namespace axi
}  // namespace verilog
}  // namespace writer
}  // namespace iroha
