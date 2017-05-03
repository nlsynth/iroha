#include "writer/verilog/axi/slave_port.h"

#include "iroha/i_design.h"
#include "writer/verilog/axi/slave_controller.h"
#include "writer/verilog/embed.h"
#include "writer/verilog/module.h"
#include "writer/verilog/table.h"

namespace iroha {
namespace writer {
namespace verilog {
namespace axi {

SlavePort::SlavePort(const IResource &res, const Table &table)
  : AxiPort(res, table) {
}

void SlavePort::BuildResource() {
  bool reset_polarity = tab_.GetModule()->GetResetPolarity();
  tab_.GetEmbeddedModules()->RequestAxiSlaveController(&res_, reset_polarity);

  BuildPortToExt();
}

void SlavePort::BuildInsn(IInsn *insn, State *st) {
}

string SlavePort::ControllerName(const IResource &res, bool reset_polarity) {
  const IResource *mem_res = res.GetParentResource();
  IArray *array = mem_res->GetArray();
  int addr_width = array->GetAddressWidth();
  return "axi_slave_controller_a" + Util::Itoa(addr_width);
}

void SlavePort::WriteController(const IResource &res,
				bool reset_polarity,
				ostream &os) {
  SlaveController c(res, reset_polarity);
  c.Write(os);
}

string SlavePort::BuildPortToExt() {
  string s;
  Module *mod = tab_.GetModule();
  SlaveController::AddPorts(mod, &s);
  for (mod = mod->GetParentModule(); mod != nullptr;
       mod = mod->GetParentModule()) {
    SlaveController::AddPorts(mod, nullptr);
  }
  return s;
}

}  // namespace axi
}  // namespace verilog
}  // namespace writer
}  // namespace iroha
