#include "writer/verilog/axi/slave_controller.h"

#include "writer/verilog/axi/slave_port.h"
#include "writer/verilog/module.h"
#include "writer/verilog/ports.h"

namespace iroha {
namespace writer {
namespace verilog {
namespace axi {  

SlaveController::SlaveController(const IResource &res,
				 bool reset_polarity)
  : AxiController(res, reset_polarity) {
  ports_.reset(new Ports);
}

SlaveController::~SlaveController() {
}

void SlaveController::Write(ostream &os) {
  string name = SlavePort::ControllerName(res_, reset_polarity_);
  os << "// slave controller: "
     << name << "\n";
  AddSramPorts();
  string initials;
  GenReadChannel(false, nullptr, ports_.get(), &initials);
  GenWriteChannel(false, nullptr, ports_.get(), &initials);
  os << "module " << name << "(";
  ports_->Output(Ports::PORT_NAME, os);
  os << ");\n";
  ports_->Output(Ports::PORT_TYPE, os);

  os << "  always @(posedge clk) begin\n"
     << "    if (" << (reset_polarity_ ? "" : "!")
     << ResetName(reset_polarity_) << ") begin\n";
  os << initials
     << "    end else begin\n";
  os << "    end\n"
     << "  end\n"
     << "endmodule\n";
}

void SlaveController::AddPorts(Module *mod, string *s) {
  Ports *ports = mod->GetPorts();
  GenWriteChannel(false, mod, ports, s);
  GenReadChannel(false, mod, ports, s);
}

}  // namespace axi
}  // namespace verilog
}  // namespace writer
}  // namespace iroha
