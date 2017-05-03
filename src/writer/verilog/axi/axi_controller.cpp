#include "writer/verilog/axi/axi_controller.h"

#include "iroha/i_design.h"
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
  const IResource *mem_res = res_.GetParentResource();
  IArray *array = mem_res->GetArray();
  addr_width_ = array->GetAddressWidth();
  data_width_ = array->GetDataType().GetWidth();
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

void AxiController::GenReadChannel(bool is_master, Module *module, Ports *ports,
				   string *s) {
  // TODO: More ports.
  AddPort("ARADDR", 32, false, is_master, module, ports, s);
  AddPort("ARVALID", 0, false, is_master, module, ports, s);
  AddPort("ARREADY", 0, true, is_master, module, ports, s);
  AddPort("ARLEN", 8, false, is_master, module, ports, s);
  AddPort("ARSIZE", 3, false, is_master, module, ports, s);

  AddPort("RVALID", 0, true, is_master, module, ports, s);
  AddPort("RDATA", 32, true, is_master, module, ports, s);
  AddPort("RREADY", 0, false, is_master, module, ports, s);
  AddPort("RLAST", 0, true, is_master, module, ports, s);
}

void AxiController::GenWriteChannel(bool is_master, Module *module,
				    Ports *ports,
				    string *s) {
  AddPort("AWADDR", 32, false, is_master, module, ports, s);
  AddPort("AWVALID", 0, false, is_master, module, ports, s);
  AddPort("AWREADY", 0, true, is_master, module, ports, s);
  AddPort("AWLEN", 8, false, is_master, module, ports, s);
  AddPort("AWSIZE", 3, false, is_master, module, ports, s);

  AddPort("WVALID", 0, false, is_master, module, ports, s);
  AddPort("WREADY", 0, true, is_master, module, ports, s);
  AddPort("WDATA", 32, false, is_master, module, ports, s);
  AddPort("WLAST", 0, false, is_master, module, ports, s);

  AddPort("BVALID", 0, true, is_master, module, ports, s);
  AddPort("BREADY", 0, false, is_master, module, ports, s);
  AddPort("BRESP", 2, true, is_master, module, ports, s);
}

void AxiController::AddPort(const string &name, int width, bool dir_s2m,
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
  ports->AddPort(name, t, width);
  string p = ", ." + name + "(" + name + ")";
  if (s != nullptr && module != nullptr) {
    *s += p;
  }
  if (module != nullptr) {
    Module *parent = module->GetParentModule();
    if (parent != nullptr) {
      ostream &os = parent->ChildModuleInstSectionStream(module);
      os << p;
    }
  }
  if (module == nullptr) {
    if (!is_input) {
      *s += "      " + name + " <= 0;\n";
    }
  }
}

void AxiController::AddSramPorts() {
  ports_->AddPort("clk", Port::INPUT_CLK, 0);
  ports_->AddPort(ResetName(reset_polarity_), Port::INPUT_RESET, 0);
  ports_->AddPort("sram_addr", Port::OUTPUT, addr_width_);
  ports_->AddPort("sram_wdata", Port::OUTPUT, data_width_);
  ports_->AddPort("sram_wen", Port::OUTPUT, 0);
  ports_->AddPort("sram_rdata", Port::INPUT, data_width_);
}

}  // namespace axi
}  // namespace verilog
}  // namespace writer
}  // namespace iroha
