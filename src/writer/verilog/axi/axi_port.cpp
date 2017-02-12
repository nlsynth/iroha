#include "writer/verilog/axi/axi_port.h"

#include "writer/module_template.h"
#include "writer/verilog/axi/controller.h"
#include "writer/verilog/embed.h"
#include "writer/verilog/module.h"
#include "writer/verilog/ports.h"
#include "writer/verilog/table.h"

namespace iroha {
namespace writer {
namespace verilog {
namespace axi {

AxiPort::AxiPort(const IResource &res, const Table &table)
  : Resource(res, table) {
}

void AxiPort::BuildResource() {
  bool reset_polarity = tab_.GetModule()->GetResetPolarity();
  tab_.GetEmbeddedModules()->RequestAxiController(&res_, reset_polarity);
  ostream &es = tmpl_->GetStream(kEmbeddedInstanceSection);
  string name = ControllerName(res_, reset_polarity);
  const string &clk = tab_.GetPorts()->GetClk();
  const string &rst = tab_.GetPorts()->GetReset();
  es << "  " << name << " inst_" << name
     << "(.clk("<< clk << "), ." << Controller::ResetName(reset_polarity)
     << "(" << rst << "));\n";
}

void AxiPort::BuildInsn(IInsn *insn, State *st) {
}

string AxiPort::ControllerName(const IResource &res, bool reset_polarity) {
  return "axi_controller";
}

void AxiPort::WriteController(const IResource &res, bool reset_polarity,
			      ostream &os) {
  Controller c(res, reset_polarity);
  c.Write(os);
}

}  // namespace axi
}  // namespace verilog
}  // namespace writer
}  // namespace iroha
