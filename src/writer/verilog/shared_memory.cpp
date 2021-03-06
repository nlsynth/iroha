#include "writer/verilog/shared_memory.h"

#include "iroha/i_design.h"
#include "iroha/logging.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"
#include "writer/connection.h"
#include "writer/module_template.h"
#include "writer/verilog/embedded_modules.h"
#include "writer/verilog/insn_writer.h"
#include "writer/verilog/internal_sram.h"
#include "writer/verilog/module.h"
#include "writer/verilog/port.h"
#include "writer/verilog/port_set.h"
#include "writer/verilog/shared_memory_accessor.h"
#include "writer/verilog/state.h"
#include "writer/verilog/table.h"
#include "writer/verilog/wire/accessor_info.h"
#include "writer/verilog/wire/wire_set.h"

namespace iroha {
namespace writer {
namespace verilog {

SharedMemory::SharedMemory(const IResource &res, const Table &table)
    : Resource(res, table) {}

void SharedMemory::BuildResource() {
  BuildMemoryResource();
  SharedMemoryAccessor::BuildMemoryAccessorResource(*this, /* do_write */ true,
                                                    /* gen_reg */ true, &res_);
}

void SharedMemory::BuildMemoryResource() {
  if (res_.GetArray()->IsExternal()) {
    BuildExternalMemoryConnection();
  } else {
    BuildMemoryInstance();
  }
  vector<const IResource *> port0_accessors;
  port0_accessors.push_back(&res_);
  auto &non_self_accessors =
      tab_.GetModule()->GetConnection().GetSharedMemoryPort0Accessors(&res_);
  for (IResource *r : non_self_accessors) {
    port0_accessors.push_back(r);
  }
  BuildAccessWireAll(port0_accessors);
  BuildAck();
}

void SharedMemory::BuildAck() {
  string rn = SharedMemory::GetName(res_);
  string ack = wire::Names::ResourceWire(rn, "ack");
  string ack_src = ack + "_src";
  ostream &rs = tab_.ResourceSectionStream();
  tab_.AddReg(ack_src, 0);
  rs << "  assign " << ack << " = " << ack_src << ";\n";
  ostream &is = tab_.InitialValueSectionStream();
  is << "      " << ack_src << " <= 0;\n";
  ostream &ss = tab_.StateOutputSectionStream();
  string req = wire::Names::ResourceWire(rn, "req");
  ss << "      " << ack_src << " <= " << req << ";\n";
}

void SharedMemory::BuildExternalMemoryConnection() {
  IArray *array = res_.GetArray();
  string prefix = "sram";
  string p = res_.GetParams()->GetPortNamePrefix();
  if (!p.empty()) {
    prefix = p;
  }
  AddPortToTop(prefix + "_addr", true, true, array->GetAddressWidth());
  AddPortToTop(prefix + "_wdata", true, true, array->GetDataType().GetWidth());
  AddPortToTop(prefix + "_wdata_en", true, true, 0);
  AddPortToTop(prefix + "_rdata", false, false,
               array->GetDataType().GetWidth());

  ostream &rs = tab_.ResourceSectionStream();
  string rn = SharedMemory::GetName(res_);
  rs << "  assign " << prefix
     << "_addr = " << wire::Names::ResourceWire(rn, "addr") << ";\n";
  rs << "  assign " << prefix
     << "_wdata = " << wire::Names::ResourceWire(rn, "wdata") << ";\n";
  rs << "  assign " << wire::Names::ResourceWire(rn, "rdata") << " = " << prefix
     << "_rdata;\n";

  FixupWen(wire::Names::ResourceWire(rn, "wen"), prefix + "_wdata_en");
}

void SharedMemory::BuildMemoryInstance() {
  int num_ports = 1;
  auto &port1_users =
      tab_.GetModule()->GetConnection().GetSharedMemoryPort1Accessors(&res_);
  if (port1_users.size() > 0) {
    // Assumes an AXI accessor.
    CHECK(port1_users.size() == 1);
    num_ports = 2;
  }
  InternalSRAM *sram = tab_.GetEmbeddedModules()->RequestInternalSRAM(
      *tab_.GetModule(), *res_.GetArray(), num_ports);
  auto *ports = tab_.GetPortSet();
  ostream &es = tmpl_->GetStream(kEmbeddedInstanceSection);
  string name = sram->GetModuleName();
  string inst = name + "_inst_" + Util::Itoa(tab_.GetITable()->GetId()) + "_" +
                Util::Itoa(res_.GetId());
  string rn = SharedMemory::GetName(res_);
  string addr_wire = wire::Names::ResourceWire(rn, "addr");
  string rdata_wire = wire::Names::ResourceWire(rn, "rdata");
  string wdata_wire = wire::Names::ResourceWire(rn, "wdata");
  string wen = MemoryWenPin(res_, 0, nullptr);
  FixupWen(wire::Names::ResourceWire(rn, "wen"), wen);

  es << "  " << name << " " << inst << "("
     << ".clk(" << ports->GetClk() << ")"
     << ", ." << sram->GetResetPinName() << "(" << ports->GetReset() << ")"
     << ", ." << sram->GetAddrPin(0) << "(" << addr_wire << ")"
     << ", ." << sram->GetRdataPin(0) << "(" << rdata_wire << ")"
     << ", ." << sram->GetWdataPin(0) << "(" << wdata_wire << ")"
     << ", ." << sram->GetWenPin(0) << "(" << wen << ")";
  if (num_ports == 2) {
    // AXI controller connects to port 1.
    es << ", ." << sram->GetAddrPin(1) << "(" << MemoryAddrPin(res_, 1, nullptr)
       << ")"
       << ", ." << sram->GetRdataPin(1) << "(" << MemoryRdataPin(res_, 1) << ")"
       << ", ." << sram->GetWdataPin(1) << "("
       << MemoryWdataPin(res_, 1, nullptr) << ")"
       << ", ." << sram->GetWenPin(1) << "(" << MemoryWenPin(res_, 1, nullptr)
       << ")";
  }
  es << ");\n";
  ostream &rs = tab_.ResourceSectionStream();
  if (num_ports == 2) {
    rs << "  wire " << sram->AddressWidthSpec() << " "
       << MemoryAddrPin(res_, 1, nullptr) << ";\n";
    rs << "  wire " << sram->DataWidthSpec() << " " << MemoryRdataPin(res_, 1)
       << ";\n";
    rs << "  wire " << sram->DataWidthSpec() << " "
       << MemoryWdataPin(res_, 1, nullptr) << ";\n";
    rs << "  wire " << MemoryWenPin(res_, 1, nullptr) << ";\n";
  }
}

void SharedMemory::FixupWen(const string &wen_wire, const string &wen) {
  // Deassert wen in the ack cycle.
  // Asserting Wen 2 cycles might not cause any problems, but
  // avoiding this to make waveforms clean.
  ostream &rs = tab_.ResourceSectionStream();
  string wen_reg = MemoryWenReg(res_, 0);
  rs << "  wire " << wen << ";\n";
  tab_.AddReg(wen_reg, 0);
  ostream &is = tab_.InitialValueSectionStream();
  is << "      " << wen_reg << " <= 0;\n";
  ostream &ss = tab_.StateOutputSectionStream();
  ss << "      " << wen_reg << " <= " << wen << ";\n";
  ostream &rvs = tab_.ResourceValueSectionStream();
  rvs << "  assign " << wen << " = " << wen_wire << " && !" << wen_reg << ";\n";
}

void SharedMemory::BuildAccessWireAll(vector<const IResource *> &accessors) {
  wire::WireSet *ws = new wire::WireSet(*this, GetName(res_));
  IArray *array = res_.GetArray();
  int aw = array->GetAddressWidth();
  int dw = array->GetDataType().GetWidth();
  for (auto *accessor : accessors) {
    auto *klass = accessor->GetClass();
    wire::AccessorInfo *ainfo = ws->AddAccessor(accessor);
    ainfo->SetDistance(accessor->GetParams()->GetDistance());
    ainfo->AddSignal("addr", wire::AccessorSignalType::ACCESSOR_WRITE_ARG, aw);
    ainfo->AddSignal("req", wire::AccessorSignalType::ACCESSOR_REQ, 0);
    ainfo->AddSignal("ack", wire::AccessorSignalType::ACCESSOR_ACK, 0);
    if (!resource::IsSharedMemoryReader(*klass)) {
      // SharedMemory, SharedMemoryWriter, AxiMaster, AxiSlave.
      wire::AccessorSignal *asig = ainfo->AddSignal(
          "wen", wire::AccessorSignalType::ACCESSOR_WRITE_ARG, 0);
      asig->sig_desc_->default0_ = true;
      ainfo->AddSignal("wdata", wire::AccessorSignalType::ACCESSOR_WRITE_ARG,
                       dw);
    }
    if (!resource::IsSharedMemoryWriter(*klass)) {
      // SharedMemory, SharedMemoryReader, AxiMaster, AxiSlave.
      ainfo->AddSignal("rdata", wire::AccessorSignalType::ACCESSOR_READ_ARG,
                       dw);
    }
  }
  ws->Build();
}

void SharedMemory::BuildInsn(IInsn *insn, State *st) {
  SharedMemoryAccessor::BuildAccessInsn(insn, st, res_, tab_);
}

string SharedMemory::GetName(const IResource &res) {
  return "mem_" + Util::Itoa(res.GetTable()->GetModule()->GetId()) + "_" +
         Util::Itoa(res.GetTable()->GetId()) + "_" + Util::Itoa(res.GetId());
}

string SharedMemory::MemoryPinPrefix(const IResource &mem,
                                     const IResource *accessor) {
  string s = GetName(mem);
  if (accessor == nullptr) {
    return s;
  }
  if (accessor != nullptr) {
    s += "_" + Util::Itoa(accessor->GetTable()->GetModule()->GetId()) + "_" +
         Util::Itoa(accessor->GetTable()->GetId()) + "_" +
         Util::Itoa(accessor->GetId());
  }
  return s;
}

string SharedMemory::MemoryRdataBuf(const IResource &res,
                                    const IResource *accessor) {
  return MemoryPinPrefix(res, accessor) + "_rbuf";
}

string SharedMemory::MemoryAddrPin(const IResource &res, int nth_port,
                                   const IResource *accessor) {
  return MemoryPinPrefix(res, accessor) + "_p" + Util::Itoa(nth_port) + "_addr";
}

string SharedMemory::MemoryRdataPin(const IResource &res, int nth_port) {
  return MemoryPinPrefix(res, nullptr) + "_p" + Util::Itoa(nth_port) + "_rdata";
}

string SharedMemory::MemoryWdataPin(const IResource &res, int nth_port,
                                    const IResource *accessor) {
  return MemoryPinPrefix(res, accessor) + "_p" + Util::Itoa(nth_port) +
         "_wdata";
}

string SharedMemory::MemoryWenPin(const IResource &res, int nth_port,
                                  const IResource *accessor) {
  return MemoryPinPrefix(res, accessor) + "_p" + Util::Itoa(nth_port) + "_wen";
}

string SharedMemory::MemoryWenReg(const IResource &res, int nth_port) {
  return MemoryPinPrefix(res, nullptr) + "_p" + Util::Itoa(nth_port) +
         "_wen_reg";
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
