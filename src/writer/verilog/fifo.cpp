#include "writer/verilog/fifo.h"

#include "iroha/i_design.h"
#include "iroha/resource_params.h"
#include "writer/module_template.h"
#include "writer/verilog/embed.h"
#include "writer/verilog/internal_sram.h"
#include "writer/verilog/module.h"
#include "writer/verilog/ports.h"
#include "writer/verilog/table.h"

namespace iroha {
namespace writer {
namespace verilog {

Fifo::Fifo(const IResource &res, const Table &table) : Resource(res, table) {
}

void Fifo::BuildResource() {
  // WIP.
  auto *params = res_.GetParams();
  IValueType data_type;
  data_type.SetWidth(params->GetWidth());
  int aw = params->GetAddrWidth();
  IArray array(nullptr, aw, data_type, false, true);
  InternalSRAM *sram =
    tab_.GetEmbeddedModules()->RequestInternalSRAM(*tab_.GetModule(),
						   array, 2);
  auto *ports = tab_.GetPorts();
  string name = sram->GetModuleName();
  string inst = name + "_inst_" + Util::Itoa(tab_.GetITable()->GetId()) +
    "_" + Util::Itoa(res_.GetId());
  ostream &es = tmpl_->GetStream(kEmbeddedInstanceSection);
  es << "  " << name << " " << inst << "("
     << ".clk(" << ports->GetClk() << ")"
     << ", ." << sram->GetResetPinName() << "(" << ports->GetReset() << ")"
    // Read.
     << ", .addr_0_i(" << ReadPtr() << "[" << (aw - 1) << ":0])"
     << ", .rdata_0_o()"
     << ", .wdata_0_i()"
     << ", .write_en_0_i()"
    // Write.
     << ", .addr_1_i(" << WritePtr() << "[" << (aw - 1) << ":0])"
     << ", .rdata_1_o()"
     << ", .wdata_1_i()"
     << ", .write_en_1_i()"
     << ");\n";

  ostream &rs = tmpl_->GetStream(kResourceSection);
  rs << "  reg [" << aw << ":0]" << ReadPtr() << ";\n"
     << "  reg [" << aw << ":0]" << WritePtr() << ";\n";

  ostream &is = tab_.InitialValueSectionStream();
  is << "      " << ReadPtr() << " <= 0;\n"
     << "      " << WritePtr() << " <= 0;\n";
}

void Fifo::BuildInsn(IInsn *insn, State *st) {
}

string Fifo::WritePtr() {
  return PinPrefix(res_, nullptr) + "_wptr";
}

string Fifo::ReadPtr() {
  return PinPrefix(res_, nullptr) + "_rptr";
}

string Fifo::PinPrefix(const IResource &res, const IResource *accessor) {
  string s = "fifo_" + Util::Itoa(res.GetTable()->GetModule()->GetId()) +
    "_" + Util::Itoa(res.GetTable()->GetId()) +
    "_" + Util::Itoa(res.GetId());
  if (accessor != nullptr) {
    s += "_" + Util::Itoa(accessor->GetTable()->GetModule()->GetId()) +
      "_" + Util::Itoa(accessor->GetTable()->GetId()) +
      "_" + Util::Itoa(accessor->GetId());
  }
  return s;
}

string Fifo::RReq(const IResource &res, const IResource *accessor) {
  return PinPrefix(res, accessor) + "_rreq";
}

string Fifo::RAck(const IResource &res, const IResource *accessor) {
  return PinPrefix(res, accessor) + "_rack";
}

string Fifo::RData(const IResource &res) {
  return PinPrefix(res, nullptr) + "_rdata";
}

string Fifo::WReq(const IResource &res, const IResource *accessor) {
  return PinPrefix(res, accessor) + "_wreq";
}

string Fifo::WAck(const IResource &res, const IResource *accessor) {
  return PinPrefix(res, accessor) + "_wack";
}

string Fifo::WData(const IResource &res, const IResource *accessor) {
  return PinPrefix(res, accessor) + "_wdata";
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
