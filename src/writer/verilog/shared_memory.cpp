#include "writer/verilog/shared_memory.h"

#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "writer/connection.h"
#include "writer/module_template.h"
#include "writer/verilog/embed.h"
#include "writer/verilog/internal_sram.h"
#include "writer/verilog/module.h"
#include "writer/verilog/ports.h"
#include "writer/verilog/table.h"

namespace iroha {
namespace writer {
namespace verilog {

SharedMemory::SharedMemory(const IResource &res, const Table &table)
  : Resource(res, table) {
}

void SharedMemory::BuildResource() {
  auto *klass = res_.GetClass();
  if (resource::IsSharedMemory(*klass)) {
    BuildMemoryResource();
  }
  if (resource::IsSharedMemoryReader(*klass)) {
    BuildMemoryReaderResource();
  }
  if (resource::IsSharedMemoryWriter(*klass)) {
    BuildMemoryWriterResource();
  }
}

void SharedMemory::BuildMemoryResource() {
  BuildMemoryInstance();
  ostream &rs = tmpl_->GetStream(kResourceSection);
  vector<const IResource *> accessors;
  auto *ext_accessors =
    tab_.GetModule()->GetConnection().GetSharedMemoryAccessors(&res_);
  accessors.push_back(&res_);
  for (IResource *r : *ext_accessors) {
    accessors.push_back(r);
  }
  ostream &is = tab_.InitialValueSectionStream();
  ostream &ss = tab_.StateOutputSectionStream();
  string high_req = "0";
  for (auto *accessor : accessors) {
    // TODO: This wire should come from actual accessor.
    rs << "  wire " << MemoryReqPin(res_, accessor) << ";\n";
    string ack = MemoryAckPin(res_, accessor);
    rs << "  reg " << ack << ";\n";
    is << "      " << ack << " <= 0;\n";
    ss << "      " << ack << " <= "
       << "!(" << high_req << ") && "
       << MemoryReqPin(res_, accessor) << ";\n";
    high_req = MemoryReqPin(res_, accessor) + " | " + high_req;
  }
  string addr_sel;
  string wdata_sel;
  string wen_sel;
  // From low to high priority.
  for (auto it = accessors.rbegin(); it != accessors.rend(); ++it) {
    const IResource *accessor = *it;
    string addr = MemoryAddrPin(res_, accessor);
    string wdata = MemoryWdataPin(res_, accessor);
    if (addr_sel.empty()) {
      addr_sel = addr;
      wdata_sel = wdata;
    } else {
      addr_sel = MemoryReqPin(res_, accessor) +
	" ? " + addr + " : (" + addr_sel + ")";
      wdata_sel = MemoryReqPin(res_, accessor) +
	" ? " + wdata + " : (" + wdata_sel + ")";
    }
    auto *klass = accessor->GetClass();
    if (resource::IsSharedMemory(*klass) ||
	resource::IsSharedMemoryWriter(*klass)) {
      string wen = MemoryWenPin(res_, accessor);
      if (wen_sel.empty()) {
	wen_sel = wen;
      } else {
	wen_sel = MemoryReqPin(res_, accessor) +
	  " ? " + wen + " : (" + wen_sel + ")";
      }
    }
  }
  string addr_wire = MemoryAddrPin(res_, nullptr);
  rs << "  assign " << addr_wire << " = " << addr_sel << ";\n";
  string wdata_wire = MemoryWdataPin(res_, nullptr);
  rs << "  assign " << wdata_wire << " = " << wdata_sel << ";\n";
  string wen_wire = MemoryWenPin(res_, nullptr);
  rs << "  assign " << wen_wire << " = " << wen_sel << ";\n";
}

void SharedMemory::BuildMemoryInstance() {
  InternalSRAM *sram =
    tab_.GetEmbeddedModules()->RequestInternalSRAM(*tab_.GetModule(), res_);
  auto *ports = tab_.GetPorts();
  ostream &es = tmpl_->GetStream(kEmbeddedInstanceSection);
  string name = sram->GetModuleName();
  string inst = name + "_inst_" + Util::Itoa(tab_.GetITable()->GetId()) +
    "_" + Util::Itoa(res_.GetId());
  string addr_wire = MemoryAddrPin(res_, nullptr);
  string rdata_wire = MemoryRdataPin(res_);
  string wdata_wire = MemoryWdataPin(res_, nullptr);
  string wen_wire = MemoryWenPin(res_, nullptr);
  es << "  " << name << " " << inst << "("
     << ".clk(" << ports->GetClk() << ")"
     << ", ." << sram->GetResetPinName() << "(" << ports->GetReset() << ")"
     << ", .addr_i(" << addr_wire << ")"
     << ", .rdata_o(" << rdata_wire << ")"
     << ", .wdata_i(" << wdata_wire << ")"
     << ", .write_en_i(" << wen_wire << ")"
     <<");\n";
  ostream &rs = tmpl_->GetStream(kResourceSection);
  rs << "  wire " << sram->AddressWidthSpec() << " " << addr_wire << ";\n";
  rs << "  wire " << sram->DataWidthSpec() << " " << rdata_wire << ";\n";
  rs << "  wire " << sram->DataWidthSpec() << " " << wdata_wire << ";\n";
  rs << "  wire " << wen_wire << ";\n";
}

void SharedMemory::BuildMemoryWriterResource() {
}

void SharedMemory::BuildMemoryReaderResource() {
}

void SharedMemory::BuildInsn(IInsn *insn, State *st) {
}

string SharedMemory::MemoryPinPrefix(const IResource &mem,
				     const IResource *accessor) {
  string s = "mem_" + Util::Itoa(mem.GetTable()->GetId()) +
    "_" + Util::Itoa(mem.GetId());
  if (accessor != nullptr) {
    s += "_" + Util::Itoa(accessor->GetTable()->GetModule()->GetId()) +
      "_" + Util::Itoa(accessor->GetTable()->GetId()) +
      "_" + Util::Itoa(accessor->GetId());
  }
  return s;
}

string SharedMemory::MemoryAddrPin(const IResource &res,
				   const IResource *accessor) {
  return MemoryPinPrefix(res, accessor) + "_addr";
}

string SharedMemory::MemoryReqPin(const IResource &res,
				  const IResource *accessor) {
  return MemoryPinPrefix(res, accessor) + "_req";
}

string SharedMemory::MemoryAckPin(const IResource &res,
				  const IResource *accessor) {
  return MemoryPinPrefix(res, accessor) + "_ack";
}

string SharedMemory::MemoryRdataPin(const IResource &res) {
  return MemoryPinPrefix(res, nullptr) + "_rdata";
}

string SharedMemory::MemoryWdataPin(const IResource &res,
				    const IResource *accessor) {
  return MemoryPinPrefix(res, accessor) + "_wdata";
}

string SharedMemory::MemoryWenPin(const IResource &res,
				  const IResource *accessor) {
  return MemoryPinPrefix(res, accessor) + "_wen";
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
