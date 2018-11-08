#include "writer/verilog/fifo.h"

#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"
#include "writer/connection.h"
#include "writer/module_template.h"
#include "writer/verilog/embed.h"
#include "writer/verilog/fifo_accessor.h"
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
  BuildController();
  BuildWires();
  BuildHandShake();
  BuildAccessConnectionsAll();
}

void Fifo::BuildWires() {

  auto *params = res_.GetParams();
  int aw = params->GetAddrWidth();
  int dw = params->GetWidth();

  ostream &rs = tab_.ResourceSectionStream();
  rs << "  reg [" << aw << ":0] " << ReadPtr() << ";\n"
     << "  reg [" << aw << ":0] " << ReadPtrBuf() << ";\n"
     << "  reg [" << aw << ":0] " << WritePtr() << ";\n"
     << "  wire " << Full() << ";\n"
     << "  wire " << Empty() << ";\n"
     << "  wire " << WEn() << ";\n"
     << "  wire [" << (dw - 1) << ":0] " << WData(res_, nullptr) << ";\n"
     << "  wire " << RReq(res_, nullptr) << ";\n"
     << "  wire " << WReq(res_, nullptr) << ";\n"
     << "  wire " << RAck(res_, nullptr) << ";\n"
     << "  wire " << WAck(res_, nullptr) << ";\n\n";

  rs << "  assign " << Empty()
     << " = (" << ReadPtr() << " == " << WritePtr() << ");\n";
  int msb = aw - 1;
  rs << "  assign " << Full()
     << " = ((" << ReadPtr() << "[" << aw << ":" << aw << "] != "
     << WritePtr() << "[" << aw << ":" << aw << "])"
     << " && (" << ReadPtr() << "[" << msb << ":" << "0] == "
     << WritePtr() << "[" << msb << ":" << "0]));\n";
  vector<string> nw_reqs;
  auto &writers = tab_.GetModule()->GetConnection().GetFifoWriters(&res_);
  for (auto *writer : writers) {
    if (FifoAccessor::UseNoWait(writer)) {
      nw_reqs.push_back(WNoWait(res_, writer));
    }
  }
  string e = WReq(res_, nullptr) + " && " + WAck(res_, nullptr);
  if (nw_reqs.size() > 0) {
    e = "(" + e + ") || (" + Util::Join(nw_reqs, " || ") + ")";
  }
  rs << "  assign " << WEn() << " = " << e << ";\n";
}

void Fifo::BuildHandShake() {
  // Reader.
  auto &readers = tab_.GetModule()->GetConnection().GetFifoReaders(&res_);
  BuildReqAckAssign(false, readers);
  // Writer.
  auto &writers = tab_.GetModule()->GetConnection().GetFifoWriters(&res_);
  BuildReqAckAssign(true, writers);
  ostream &rs = tab_.ResourceSectionStream();
  string s;
  for (auto *writer : writers) {
    if (FifoAccessor::UseNoWait(writer)) {
      if (s.empty()) {
	s = WData(res_, writer);
      } else {
	s = WNoWait(res_, writer) + " ? " + WData(res_, writer) +
	  " : (" + s + ")";
      }
    }
  }
  for (auto *writer : writers) {
    if (s.empty()) {
      s = WData(res_, writer);
    } else {
      s = WReq(res_, writer) + " ? " + WData(res_, writer) + " : (" + s + ")";
    }
  }
  rs << "  assign " << WData(res_, nullptr) << " = " << s << ";\n";
}

void Fifo::BuildReqAckAssign(bool is_write,
			     const vector<IResource *> &accessors) {
  vector<string> acks;
  vector<string> reqs;
  for (auto *accessor : accessors) {
    if (is_write) {
      acks.push_back(WAck(res_, accessor));
      reqs.push_back(WReq(res_, accessor));
    } else {
      acks.push_back(RAck(res_, accessor));
      reqs.push_back(RReq(res_, accessor));
    }
  }
  ostream &rs = tab_.ResourceSectionStream();
  if (is_write) {
    AssignJoin(WReq(res_, nullptr), reqs, rs);
  } else {
    AssignJoin(RReq(res_, nullptr), reqs, rs);
  }
  if (is_write) {
    AssignJoin(WAck(res_, nullptr), acks, rs);
  } else {
    AssignJoin(RAck(res_, nullptr), acks, rs);
  }
}

void Fifo::AssignJoin(const string &lhs, const vector<string> &accessors,
		      ostream &os) {
  os << "  assign " << lhs << " = ";
  if (accessors.size() == 0) {
    os << "0;\n";
  } else {
    os << Util::Join(accessors, " | ") << ";\n";
  }
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
     << ", .addr_0_i(" << ReadPtrBuf() << "[" << (aw - 1) << ":0])"
     << ", .rdata_0_o(" << RData(res_) << ")"
     << ", .wdata_0_i(/*not connected*/)"
     << ", .write_en_0_i(1'b0)"
    // Write.
     << ", .addr_1_i(" << WritePtr() << "[" << (aw - 1) << ":0])"
     << ", .rdata_1_o(/*not connected*/)"
     << ", .wdata_1_i(" << WData(res_, nullptr) << ")"
     << ", .write_en_1_i(" << WEn() << ")"
     << ");\n";
}

void Fifo::BuildInsn(IInsn *insn, State *st) {
}

void Fifo::BuildAccessConnectionsAll() {
  InterModuleWire wire(*this);
  int dw = res_.GetParams()->GetWidth();
  auto &readers = tab_.GetModule()->GetConnection().GetFifoReaders(&res_);
  // This is a kludge. We might change the method above..
  vector<const IResource *> readers_const;
  for (auto *reader : readers) {
    bool drive_req_by_reg = true;
    if (resource::IsDataFlowIn(*(reader->GetClass()))) {
      drive_req_by_reg = false;
    }
    wire.AddWire(*reader, RReq(res_, reader), 0, false, drive_req_by_reg);
    wire.AddWire(*reader, RAck(res_, reader), 0, true, true);
    readers_const.push_back(reader);
  }
  // Driven from sram module.
  wire.AddSharedWires(readers_const, RData(res_), dw, true, false);
  auto &writers = tab_.GetModule()->GetConnection().GetFifoWriters(&res_);
  for (auto *writer : writers) {
    wire.AddWire(*writer, WReq(res_, writer), 0, false, true);
    wire.AddWire(*writer, WAck(res_, writer), 0, true, true);
    wire.AddWire(*writer, WData(res_, writer), dw, false, true);
    if (FifoAccessor::UseNoWait(writer)) {
      wire.AddWire(*writer, WNoWait(res_, writer), 0, false, true);
    }
  }
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

void Fifo::BuildController() {
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
     << "      if (" << WReq(res_, nullptr) << " && !" << Full()
     << " && !" << WAck(res_, nullptr) << ") begin\n";
  auto &writers = tab_.GetModule()->GetConnection().GetFifoWriters(&res_);
  BuildAck(true, writers);
  es << "      end else begin\n";
  BuildAckAssigns(true, "", writers, "        ", es);
  es << "      end\n"
     << "      if (" << RReq(res_, nullptr) << " && !" << Empty()
     << " && !" << RAck(res_, nullptr) << ") begin\n"
     << "        " << ReadPtr() << " <= " << ReadPtr() << " + 1;\n"
     << "        " << ReadPtrBuf() << " <= " << ReadPtr() << ";\n";
  auto &readers = tab_.GetModule()->GetConnection().GetFifoReaders(&res_);
  BuildAck(false, readers);
  es << "      end else begin\n";
  BuildAckAssigns(false, "", readers, "        ", es);
  es << "      end\n"
     << "    end\n"
     << "  end\n";
}

void Fifo::BuildAck(bool is_write, const vector<IResource *> &accessors) {
  ostream &es = tmpl_->GetStream(kEmbeddedInstanceSection);
  bool is_first = true;
  // Use reverse order to align with req ternary operators.
  for (auto it = accessors.rbegin(); it != accessors.rend(); ++it) {
    auto *accessor = *it;
    string req;
    string ack;
    if (is_write) {
      req = WReq(res_, accessor);
      ack = WAck(res_, accessor);
    } else {
      req = RReq(res_, accessor);
      ack = RAck(res_, accessor);
    }
    if (is_first) {
      es << "        ";
    } else {
      es << " else ";
    }
    es << "if (" << req << ") begin\n";
    BuildAckAssigns(is_write, ack, accessors, "          ", es);
    es << "        end";
    is_first = false;
  }
  es << "\n";
}

void Fifo::BuildAckAssigns(bool is_write, const string &ack,
			   const vector<IResource *> &accessors,
			   const string &indent,
			   ostream &os) {
  for (auto *accessor : accessors) {
    string a;
    if (is_write) {
      a = WAck(res_, accessor);
    } else {
      a = RAck(res_, accessor);
    }
    os << indent << a << " <= ";
    if (ack == a) {
      os << "1";
    } else {
      os << "0";
    }
    os << ";\n";
  }
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

string Fifo::WNoWait(const IResource &res, const IResource *accessor) {
  return PinPrefix(res, accessor) + "_wno_wait";
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
