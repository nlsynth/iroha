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
  os << "module " << name << "(";
  ports_->Output(Ports::PORT_NAME, os);
  os << ");\n";
  ports_->Output(Ports::PORT_TYPE, os);
  os << "  always @(posedge clk) begin\n"
     << "    if (" << (reset_polarity_ ? "" : "!")
     << ResetName(reset_polarity_) << ") begin\n"
     << "      wen <= 0;\n"
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

}  // namespace axi
}  // namespace verilog
}  // namespace writer
}  // namespace iroha
