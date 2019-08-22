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

void AxiController::WriteModuleHeader(const string &name, ostream &os) {
  string guard = name + "_defined";
  os << "`ifndef " << guard << "\n"
     << " `define " << guard << "\n";
  os << "module " << name << "(";
}

void AxiController::WriteModuleFooter(const string &name, ostream &os) {
  string guard = name + "_defined";
  os << "endmodule\n"
     << "`endif  // " << guard << "\n";
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
