#include "writer/verilog/axi/slave_port.h"

#include "iroha/i_design.h"
#include "writer/module_template.h"
#include "writer/verilog/axi/slave_controller.h"
#include "writer/verilog/embed.h"
#include "writer/verilog/module.h"
#include "writer/verilog/ports.h"
#include "writer/verilog/shared_memory.h"
#include "writer/verilog/table.h"

namespace iroha {
namespace writer {
namespace verilog {
namespace axi {

SlavePort::SlavePort(const IResource &res, const Table &table)
  : AxiPort(res, table) {
}

void SlavePort::BuildResource() {
  string wires = BuildPortToExt();
  BuildControllerInstance(wires);
  if (!IsExclusiveAccessor()) {
    SharedMemory::BuildMemoryAccessorResource(*this, true, false,
					      res_.GetParentResource());
  }
}

void SlavePort::BuildInsn(IInsn *insn, State *st) {
}

string SlavePort::ControllerName(const IResource &res, bool reset_polarity) {
  const IResource *mem_res = res.GetParentResource();
  IArray *array = mem_res->GetArray();
  int addr_width = array->GetAddressWidth();
  return "axi_slave_controller_a" + Util::Itoa(addr_width) +
    "d" + Util::Itoa(array->GetDataType().GetWidth());
}

void SlavePort::WriteController(const IResource &res,
				bool reset_polarity,
				ostream &os) {
  SlaveController c(res, reset_polarity);
  c.Write(os);
}

void SlavePort::BuildControllerInstance(const string &wires) {
  tab_.GetEmbeddedModules()->RequestAxiSlaveController(&res_, reset_polarity_);
  ostream &es = tmpl_->GetStream(kEmbeddedInstanceSection);
  string name = ControllerName(res_, reset_polarity_);
  const string &clk = tab_.GetPorts()->GetClk();
  const string &rst = tab_.GetPorts()->GetReset();
  const IResource *mem = res_.GetParentResource();
  es << "  " << name << " inst_" << PortSuffix()
     << "_" << name << "(";
  OutputSRAMConnection(es);
  es << wires
     << ");\n";
}

string SlavePort::BuildPortToExt() {
  string s;
  Module *mod = tab_.GetModule();
  PortConfig cfg = AxiPort::GetPortConfig(res_);
  SlaveController::AddPorts(cfg, mod, &s);
  for (mod = mod->GetParentModule(); mod != nullptr;
       mod = mod->GetParentModule()) {
    SlaveController::AddPorts(cfg, mod, nullptr);
  }
  return s;
}

}  // namespace axi
}  // namespace verilog
}  // namespace writer
}  // namespace iroha
