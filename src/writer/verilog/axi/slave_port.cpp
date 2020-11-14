#include "writer/verilog/axi/slave_port.h"

#include "iroha/i_design.h"
#include "writer/module_template.h"
#include "writer/verilog/axi/slave_controller.h"
#include "writer/verilog/embedded_modules.h"
#include "writer/verilog/insn_writer.h"
#include "writer/verilog/module.h"
#include "writer/verilog/port.h"
#include "writer/verilog/shared_memory_accessor.h"
#include "writer/verilog/state.h"
#include "writer/verilog/table.h"

namespace iroha {
namespace writer {
namespace verilog {
namespace axi {

SlavePort::SlavePort(const IResource &res, const Table &table)
    : AxiPort(res, table) {}

void SlavePort::BuildResource() {
  string wires_to_ext = BuildPortToExt();
  BuildControllerInstance(wires_to_ext);
  if (!IsExclusiveAccessor()) {
    SharedMemoryAccessor::BuildMemoryAccessorResource(*this, true, false,
                                                      res_.GetParentResource());
  }

  ostream &os = tab_.ResourceSectionStream();
  os << "  wire " << NotifyWire() << ";\n"
     << "  reg " << NotifyAckWire() << ";\n";

  ostream &is = tab_.InitialValueSectionStream();
  is << "      " << NotifyAckWire() << " <= 0;\n";
}

void SlavePort::BuildInsn(IInsn *insn, State *st) {
  static const char I[] = "          ";
  string insn_st = InsnWriter::MultiCycleStateName(*(insn->GetResource()));
  ostream &os = st->StateBodySectionStream();
  os << I << "// AXI notification wait\n"
     << I << "if (" << insn_st << " == 0) begin\n"
     << I << "  if (" << NotifyWire() << ") begin\n"
     << I << "    " << NotifyAckWire() << " <= 1;\n"
     << I << "    " << insn_st << " <= 1;\n"
     << I << "  end\n"
     << I << "end\n"
     << I << "if (" << insn_st << " == 1) begin\n"
     << I << "  if (!" << NotifyWire() << ") begin\n"
     << I << "    " << NotifyAckWire() << " <= 0;\n"
     << I << "    " << insn_st << " <= 3;\n"
     << I << "  end\n"
     << I << "end\n";
}

string SlavePort::ControllerName(const IResource &res) {
  const IResource *mem_res = res.GetParentResource();
  IArray *array = mem_res->GetArray();
  int addr_width = array->GetAddressWidth();
  return "axi_slave_controller_a" + Util::Itoa(addr_width) + "d" +
         Util::Itoa(array->GetDataType().GetWidth());
}

void SlavePort::WriteController(const IResource &res, bool reset_polarity,
                                ostream &os) {
  SlaveController c(res, reset_polarity);
  c.Write(os);
}

void SlavePort::BuildControllerInstance(const string &wires_to_ext) {
  tab_.GetEmbeddedModules()->RequestAxiSlaveController(&res_);
  ostream &es = tmpl_->GetStream(kEmbeddedInstanceSection);
  string name = ControllerName(res_);
  es << "  " << name << " inst_" << PortSuffix() << "_" << name << "(";
  OutputSRAMConnection(es);
  OutputNotifierConnection(es);
  es << wires_to_ext << ");\n";
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

void SlavePort::OutputNotifierConnection(ostream &os) {
  os << ", .access_notify(" << NotifyWire() << "), .access_ack("
     << NotifyAckWire() << ")";
}

string SlavePort::NotifyWire() { return "access_notify" + PortSuffix(); }

string SlavePort::NotifyAckWire() { return "access_ack" + PortSuffix(); }

}  // namespace axi
}  // namespace verilog
}  // namespace writer
}  // namespace iroha
