#include "writer/verilog/fifo.h"

#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"
#include "writer/connection.h"
#include "writer/module_template.h"
#include "writer/verilog/embed.h"
#include "writer/verilog/fifo_accessor.h"
#include "writer/verilog/internal_sram.h"
#include "writer/verilog/module.h"
#include "writer/verilog/ports.h"
#include "writer/verilog/port_set.h"
#include "writer/verilog/table.h"
#include "writer/verilog/wire/accessor_info.h"
#include "writer/verilog/wire/wire_set.h"

// TODO: Fix code no_wait writers.

namespace iroha {
namespace writer {
namespace verilog {

Fifo::Fifo(const IResource &res, const Table &table) : Resource(res, table) {
}

void Fifo::BuildResource() {
  BuildMemoryInstance();
  BuildWires();
  BuildController();
  BuildWriterConnections();
  BuildReaderConnections();
}

void Fifo::BuildWires() {
  auto *params = res_.GetParams();
  int aw = params->GetAddrWidth();

  // Use [aw:0] (instead of aw-1) to have an extra bit to handle full.
  tab_.AddReg(ReadPtr(), aw + 1);
  tab_.AddReg(ReadPtrBuf(), aw + 1);
  tab_.AddReg(WritePtr(), aw + 1);
  tab_.AddReg(RAckReg(), 0);
  tab_.AddReg(WAckReg(), 0);
  ostream &rs = tab_.ResourceSectionStream();
  rs << "  wire " << Full() << ";\n"
     << "  wire " << Empty() << ";\n"
     << "  wire " << WEn() << ";\n\n";

  rs << "  assign " << Empty()
     << " = (" << ReadPtr() << " == " << WritePtr() << ");\n";
  int msb = aw - 1;
  rs << "  assign " << Full()
     << " = ((" << ReadPtr() << "[" << aw << ":" << aw << "] != "
     << WritePtr() << "[" << aw << ":" << aw << "])"
     << " && (" << ReadPtr() << "[" << msb << ":" << "0] == "
     << WritePtr() << "[" << msb << ":" << "0]));\n";
  bool has_no_wait = false;
  auto &writers = tab_.GetModule()->GetConnection().GetFifoWriters(&res_);
  for (auto *writer : writers) {
    if (FifoAccessor::UseNoWait(writer)) {
      has_no_wait = true;
    }
  }
  string e = WReqWire() + " && " + WAckWire();
  if (has_no_wait) {
    e = "(" + e + ") || " + WNoWaitWire();
  }
  rs << "  assign " << WEn() << " = " << e << ";\n";
  rs << "  assign " << WAckWire() << " = " << WAckReg() << ";\n";
  rs << "  assign " << RAckWire() << " = " << RAckReg() << ";\n";
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
  auto *ports = tab_.GetPortSet();
  string name = sram->GetModuleName();
  string inst = name + "_inst_" + Util::Itoa(tab_.GetITable()->GetId()) +
    "_" + Util::Itoa(res_.GetId());
  ostream &es = tmpl_->GetStream(kEmbeddedInstanceSection);
  es << "  " << name << " " << inst << "("
     << ".clk(" << ports->GetClk() << ")"
     << ", ." << sram->GetResetPinName() << "(" << ports->GetReset() << ")"
    // Read.
     << ", .addr_0_i(" << ReadPtrBuf() << "[" << (aw - 1) << ":0])"
     << ", .rdata_0_o(" << RDataWire() << ")"
     << ", .wdata_0_i(/*not connected*/)"
     << ", .write_en_0_i(1'b0)"
    // Write.
     << ", .addr_1_i(" << WritePtr() << "[" << (aw - 1) << ":0])"
     << ", .rdata_1_o(/*not connected*/)"
     << ", .wdata_1_i(" << WDataWire() << ")"
     << ", .write_en_1_i(" << WEn() << ")"
     << ");\n";
}

void Fifo::BuildInsn(IInsn *insn, State *st) {
}

void Fifo::BuildWriterConnections() {
  auto &writers = tab_.GetModule()->GetConnection().GetFifoWriters(&res_);
  wire::WireSet *ws = new wire::WireSet(*this, GetNameRW(res_, true));
  int dw = res_.GetParams()->GetWidth();
  for (auto *writer : writers) {
    wire::AccessorInfo *ainfo = ws->AddAccessor(writer);
    ainfo->SetDistance(writer->GetParams()->GetDistance());
    ainfo->AddSignal("wreq", wire::AccessorSignalType::ACCESSOR_REQ, 0);
    ainfo->AddSignal("wack", wire::AccessorSignalType::ACCESSOR_ACK, 0);
    ainfo->AddSignal("wdata", wire::AccessorSignalType::ACCESSOR_WRITE_ARG, dw);
    if (FifoAccessor::UseNoWait(writer)) {
      ainfo->AddSignal("wno_wait",
		       wire::AccessorSignalType::ACCESSOR_NOTIFY_PARENT, 0);
    }
  }
  ws->Build();
}

void Fifo::BuildReaderConnections() {
  auto &readers = tab_.GetModule()->GetConnection().GetFifoReaders(&res_);
  wire::WireSet *ws = new wire::WireSet(*this, GetNameRW(res_, false));
  int dw = res_.GetParams()->GetWidth();
  for (auto *reader : readers) {
    wire::AccessorInfo *ainfo = ws->AddAccessor(reader);
    ainfo->SetDistance(reader->GetParams()->GetDistance());
    ainfo->AddSignal("rreq", wire::AccessorSignalType::ACCESSOR_REQ, 0);
    ainfo->AddSignal("rack", wire::AccessorSignalType::ACCESSOR_ACK, 0);
    ainfo->AddSignal("rdata", wire::AccessorSignalType::ACCESSOR_READ_ARG, dw);
  }
  ws->Build();
}

string Fifo::WritePtr() {
  return PinPrefix(res_, nullptr) + "_wptr";
}

string Fifo::ReadPtr() {
  return PinPrefix(res_, nullptr) + "_rptr";
}

string Fifo::ReadPtrBuf() {
  return ReadPtr() + "_buf";
}

string Fifo::WEn() {
  return PinPrefix(res_, nullptr) + "_wen";
}

string Fifo::WReqWire() {
  string wrn = GetNameRW(res_, true);
  return wire::Names::ResourceWire(wrn, "wreq");
}

string Fifo::WAckWire() {
  string wrn = GetNameRW(res_, true);
  return wire::Names::ResourceWire(wrn, "wack");
}

string Fifo::WAckReg() {
  string wrn = GetNameRW(res_, true);
  return wire::Names::ResourceSignalBase(wrn, "wack") + "_reg";
}

string Fifo::WDataWire() {
  string wrn = GetNameRW(res_, true);
  return wire::Names::ResourceWire(wrn, "wdata");
}

string Fifo::WNoWaitWire() {
  string wrn = GetNameRW(res_, true);
  return wire::Names::ResourceWire(wrn, "wno_wait");
}

string Fifo::RReqWire() {
  string rrn = GetNameRW(res_, false);
  return wire::Names::ResourceWire(rrn, "rreq");
}

string Fifo::RAckWire() {
  string rrn = GetNameRW(res_, false);
  return wire::Names::ResourceWire(rrn, "rack");
}

string Fifo::RAckReg() {
  string rrn = GetNameRW(res_, false);
  return wire::Names::ResourceSignalBase(rrn, "rack") + "_reg";
}

string Fifo::RDataWire() {
  string rrn = GetNameRW(res_, false);
  return wire::Names::ResourceWire(rrn, "rdata");
}

void Fifo::BuildController() {
  string wreq = WReqWire();
  ostream &es = tmpl_->GetStream(kEmbeddedInstanceSection);
  bool reset_polarity = tab_.GetModule()->GetResetPolarity();
  es << "  // fifo controller.\n";
  es << "  always @(posedge clk) begin\n"
     << "    if (" << (reset_polarity ? "rst" : "!rst_n")
     << ") begin\n"
     << "      " << ReadPtr() << " <= 0;\n"
     << "      " << WritePtr() << " <= 0;\n"
     << "    end else begin\n"
     << "      if (" << WEn() << ") begin\n"
     << "        " << WritePtr() << " <= " << WritePtr() << " + 1;\n"
     << "      end\n"
     << "      if (" << wreq << " && !" << Full()
     << " && !" << WAckWire() << ") begin\n";
  es << "        if (" << wreq << ") begin\n"
     << "          " << WAckReg() << " <= 1;\n"
     << "        end\n";
  es << "      end else begin\n";
  es << "        " << WAckReg() << " <= 0;\n";
  es << "      end\n";
  es << "      if (" << RReqWire() << " && !" << Empty()
     << " && !" << RAckWire() << ") begin\n"
     << "        " << ReadPtr() << " <= " << ReadPtr() << " + 1;\n"
     << "        " << ReadPtrBuf() << " <= " << ReadPtr() << ";\n";
  es << "        " << RAckReg() << " <= " << "1;\n";
  es << "      end else begin\n"
     << "        " << RAckReg() << " <= " << "0;\n";
  es << "      end\n"
     << "    end\n"
     << "  end\n";
}

string Fifo::PinPrefix(const IResource &res, const IResource *accessor) {
  if (accessor == nullptr) {
    return GetName(res);
  }
  auto *klass = accessor->GetClass();
  string s = GetNameRW(res, resource::IsFifoWriter(*klass));
  s += "_" + Util::Itoa(accessor->GetTable()->GetModule()->GetId()) +
    "_" + Util::Itoa(accessor->GetTable()->GetId()) +
    "_" + Util::Itoa(accessor->GetId());
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

string Fifo::WNoWait(const IResource &res, const IResource *accessor) {
  return PinPrefix(res, accessor) + "_wno_wait";
}

string Fifo::GetName(const IResource &res) {
  return "fifo_" + Util::Itoa(res.GetTable()->GetModule()->GetId()) +
    "_" + Util::Itoa(res.GetTable()->GetId()) +
    "_" + Util::Itoa(res.GetId());
}

string Fifo::GetNameRW(const IResource &res, bool is_write) {
  string s = GetName(res);
  if (is_write) {
    return s + "_w";
  } else {
    return s + "_r";
  }
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
