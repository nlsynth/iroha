#include "writer/verilog/axi/controller.h"

#include "iroha/i_design.h"
#include "writer/verilog/axi/axi_port.h"
#include "writer/verilog/module.h"
#include "writer/verilog/ports.h"
#include "writer/verilog/table.h"

namespace iroha {
namespace writer {
namespace verilog {
namespace axi {

Controller::Controller(const IResource &res, bool reset_polarity)
  : res_(res), reset_polarity_(reset_polarity) {
  ports_.reset(new Ports);
}

Controller::~Controller() {
}

void Controller::Write(ostream &os) {
  const IResource *mem_res = res_.GetSharedRegister();
  IArray *array = mem_res->GetArray();
  int addr_width = array->GetAddressWidth();
  int data_width = array->GetDataType().GetWidth();
  string name = AxiPort::ControllerName(res_, reset_polarity_);
  ports_->AddPort("clk", Port::INPUT_CLK, 0);
  ports_->AddPort(ResetName(reset_polarity_), Port::INPUT_RESET, 0);
  ports_->AddPort("addr", Port::OUTPUT, addr_width);
  ports_->AddPort("wdata", Port::OUTPUT, data_width);
  ports_->AddPort("wen", Port::OUTPUT, 0);
  ports_->AddPort("rdata", Port::INPUT, data_width);
  bool r, w;
  AxiPort::GetReadWrite(res_, &r, &w);
  string initials;
  if (r) {
    GenReadChannel(nullptr, ports_.get(), &initials);
  }
  if (w) {
    GenWriteChannel(nullptr, ports_.get(), &initials);
  }
  os << "module " << name << "(";
  ports_->Output(Ports::PORT_NAME, os);
  os << ");\n";
  ports_->Output(Ports::PORT_TYPE, os);
  os << "  always @(posedge clk) begin\n"
     << "    if (" << (reset_polarity_ ? "" : "!")
     << ResetName(reset_polarity_) << ") begin\n"
     << "      wen <= 0;\n"
     << initials
     << "    end\n";
  os << "  end\n"
     << "endmodule\n";
}

string Controller::ResetName(bool polarity) {
  if (polarity) {
    return "rst";
  } else {
    return "rst_n";
  }
}

void Controller::AddPorts(Module *mod, bool r, bool w,
			  string *s) {
  Ports *ports = mod->GetPorts();
  if (r) {
    GenReadChannel(mod, ports, s);
  }
  if (w) {
    GenWriteChannel(mod, ports, s);
  }
}

void Controller::GenReadChannel(Module *module, Ports *ports,
				string *s) {
  // TODO: More ports.
  AddPort("ARVALID", 0, true, module, ports, s);
  AddPort("ARREADY", 0, false, module, ports, s);
}

void Controller::GenWriteChannel(Module *module, Ports *ports,
				 string *s) {
  AddPort("AWVALID", 0, true, module, ports, s);
  AddPort("AWREADY", 0, false, module, ports, s);
}

void Controller::AddPort(const string &name, int width, bool dir,
			 Module *module, Ports *ports,
			 string *s) {
  Port::PortType t;
  if (dir) {
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
  if (s != nullptr) {
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
    if (!dir) {
      *s = "      " + name + " <= 0;\n";
    }
  }
}

}  // namespace axi
}  // namespace verilog
}  // namespace writer
}  // namespace iroha
