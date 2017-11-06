#include "writer/verilog/fifo.h"

#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"
#include "writer/connection.h"
#include "writer/module_template.h"
#include "writer/verilog/embed.h"
#include "writer/verilog/internal_sram.h"
#include "writer/verilog/inter_module_wire.h"
#include "writer/verilog/module.h"
#include "writer/verilog/ports.h"
#include "writer/verilog/table.h"

namespace iroha {
namespace writer {
namespace verilog {

Fifo::Fifo(const IResource &res, const Table &table) : Resource(res, table) {
}

void Fifo::BuildResource() {
  BuildMemoryInstance();
  BuildWires();
  BuildAccessConnectionsAll();
}

void Fifo::BuildWires() {

  auto *params = res_.GetParams();
  int aw = params->GetAddrWidth();
  int dw = params->GetWidth();

  ostream &rs = tmpl_->GetStream(kResourceSection);
  rs << "  reg [" << aw << ":0] " << ReadPtr() << ";\n"
     << "  reg [" << aw << ":0] " << WritePtr() << ";\n"
     << "  wire " << Full() << ";\n"
     << "  wire " << Empty() << ";\n"
     << "  wire [" << (dw - 1) << ":0] " << WData(res_, nullptr) << ";\n"
     << "  wire " << WReq(res_, nullptr) << ";\n\n";

  rs << "  assign " << Empty()
     << " = (" << ReadPtr() << " == " << WritePtr() << ");\n";
  int msb = aw - 1;
  rs << "  assign " << Full()
     << " = ((" << ReadPtr() << "[" << aw << ":" << aw << "] != "
     << WritePtr() << "[" << aw << ":" << aw << "])"
     << " && (" << ReadPtr() << "[" << msb << ":" << "0] == "
     << WritePtr() << "[" << msb << ":" << "0]));\n";

  ostream &is = tab_.InitialValueSectionStream();
  is << "      " << ReadPtr() << " <= 0;\n"
     << "      " << WritePtr() << " <= 0;\n";
}

void Fifo::BuildMemoryInstance() {
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
     << ", .rdata_0_o(" << RData(res_) << ")"
     << ", .wdata_0_i(/*not connected*/)"
     << ", .write_en_0_i(0)"
    // Write.
     << ", .addr_1_i(" << WritePtr() << "[" << (aw - 1) << ":0])"
     << ", .rdata_1_o(/*not connected*/)"
     << ", .wdata_1_i(" << WData(res_, nullptr) << ")"
     << ", .write_en_1_i(" << WReq(res_, nullptr) << ")"
     << ");\n";
}

void Fifo::BuildInsn(IInsn *insn, State *st) {
}

void Fifo::BuildAccessConnectionsAll() {
  auto &readers = tab_.GetModule()->GetConnection().GetFifoReaders(&res_);
  for (auto *reader : readers) {
    BuildAccessConnection(reader);
  }
  auto &writers = tab_.GetModule()->GetConnection().GetFifoWriters(&res_);
  for (auto *writer : writers) {
    BuildAccessConnection(writer);
  }
}

void Fifo::BuildAccessConnection(IResource *accessor) {
  InterModuleWire wire(*this);
  IResource *fifo = accessor->GetParentResource();
  bool is_reader = resource::IsFifoReader(*(accessor->GetClass()));
  auto *params = fifo->GetParams();
  int dw = params->GetWidth();
  if (is_reader) {
    wire.AddWire(*accessor, RReq(*fifo, accessor), 0, false, true);
    wire.AddWire(*accessor, RAck(*fifo, accessor), 0, true, true);
    // TODO: Make this shared between accessors.
    wire.AddWire(*accessor, RData(*fifo), dw, false, false);
  } else {
    wire.AddWire(*accessor, WReq(*fifo, accessor), 0, false, true);
    wire.AddWire(*accessor, WAck(*fifo, accessor), 0, true, true);
    wire.AddWire(*accessor, WData(*fifo, accessor), dw, false, true);
  }
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

string Fifo::RDataBuf(const IResource &reader) {
  return PinPrefix(reader, nullptr) + "_rdata_buf";
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

string Fifo::Full() {
  return PinPrefix(res_, nullptr) + "_full";
}

string Fifo::Empty() {
  return PinPrefix(res_, nullptr) + "_empty";
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
