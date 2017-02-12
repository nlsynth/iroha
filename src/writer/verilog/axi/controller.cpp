#include "writer/verilog/axi/controller.h"

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
  string name = AxiPort::ControllerName(res_, reset_polarity_);
  ports_->AddPort("clk", Port::INPUT_CLK, 0);
  ports_->AddPort(ResetName(reset_polarity_), Port::INPUT_RESET, 0);
  os << "module " << name << "(";
  ports_->Output(Ports::PORT_NAME, os);
  os << ");\n";
  ports_->Output(Ports::PORT_TYPE, os);
  os << "endmodule\n";
}

string Controller::ResetName(bool polarity) {
  if (polarity) {
    return "rst";
  } else {
    return "rst_n";
  }
}

}  // namespace axi
}  // namespace verilog
}  // namespace writer
}  // namespace iroha
