#include "writer/verilog/axi/axi_controller.h"

#include "writer/verilog/module.h"
#include "writer/verilog/ports.h"

namespace iroha {
namespace writer {
namespace verilog {
namespace axi {

AxiController::AxiController(const IResource &res,
			     bool reset_polarity)
  : res_(res), reset_polarity_(reset_polarity) {
}

string AxiController::ResetName(bool polarity) {
  if (polarity) {
    return "rst";
  } else {
    return "rst_n";
  }
}

void AxiController::GenReadChannel(Module *module, Ports *ports,
				   string *s) {
  // TODO: More ports.
  AddPort("ARADDR", 32, false, module, ports, s);
  AddPort("ARVALID", 0, false, module, ports, s);
  AddPort("ARREADY", 0, true, module, ports, s);
  AddPort("ARLEN", 8, false, module, ports, s);
  AddPort("ARSIZE", 3, false, module, ports, s);

  AddPort("RVALID", 0, true, module, ports, s);
  AddPort("RDATA", 32, true, module, ports, s);
  AddPort("RREADY", 0, false, module, ports, s);
  AddPort("RLAST", 0, true, module, ports, s);
}

void AxiController::GenWriteChannel(Module *module, Ports *ports,
				    string *s) {
  AddPort("AWADDR", 32, false, module, ports, s);
  AddPort("AWVALID", 0, false, module, ports, s);
  AddPort("AWREADY", 0, true, module, ports, s);
  AddPort("AWLEN", 8, false, module, ports, s);
  AddPort("AWSIZE", 3, false, module, ports, s);

  AddPort("WVALID", 0, false, module, ports, s);
  AddPort("WREADY", 0, true, module, ports, s);
  AddPort("WDATA", 32, false, module, ports, s);
  AddPort("WLAST", 0, false, module, ports, s);

  AddPort("BVALID", 0, true, module, ports, s);
  AddPort("BREADY", 0, false, module, ports, s);
  AddPort("BRESP", 2, true, module, ports, s);
}

void AxiController::AddPort(const string &name, int width, bool dir_s2m,
			    Module *module, Ports *ports,
			    string *s) {
  Port::PortType t;
  if (dir_s2m) {
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
    if (!dir_s2m) {
      *s += "      " + name + " <= 0;\n";
    }
  }
}

}  // namespace axi
}  // namespace verilog
}  // namespace writer
}  // namespace iroha
