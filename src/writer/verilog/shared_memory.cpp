#include "writer/verilog/shared_memory.h"

#include "iroha/i_design.h"
#include "iroha/logging.h"
#include "iroha/resource_class.h"
#include "writer/connection.h"
#include "writer/module_template.h"
#include "writer/verilog/embed.h"
#include "writer/verilog/insn_writer.h"
#include "writer/verilog/internal_sram.h"
#include "writer/verilog/module.h"
#include "writer/verilog/ports.h"
#include "writer/verilog/state.h"
#include "writer/verilog/table.h"
#include "writer/verilog/wire/inter_module_wire.h"

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
    BuildMemoryAccessorResource(*this, true, true, &res_);
  }
  if (resource::IsSharedMemoryReader(*klass)) {
    BuildMemoryAccessorResource(*this, false, true, res_.GetParentResource());
  }
  if (resource::IsSharedMemoryWriter(*klass)) {
    BuildMemoryAccessorResource(*this, true, true, res_.GetParentResource());
  }
}

void SharedMemory::BuildMemoryResource() {
  if (res_.GetArray()->IsExternal()) {
    BuildExternalMemoryConnection();
  } else {
    BuildMemoryInstance();
  }
  vector<const IResource *> accessors;
  accessors.push_back(&res_);
  auto &ext_accessors =
    tab_.GetModule()->GetConnection().GetSharedMemoryAccessors(&res_);
  for (IResource *r : ext_accessors) {
    accessors.push_back(r);
  }
  BuildAccessWireAll(accessors);
  ostream &is = tab_.InitialValueSectionStream();
  ostream &ss = tab_.StateOutputSectionStream();
  string high_req = "0";
  for (auto *accessor : accessors) {
    string ack = MemoryAckPin(res_, accessor);
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
    auto *klass = accessor->GetClass();
    bool may_write = false;
    if (resource::IsSharedMemory(*klass) ||
	resource::IsSharedMemoryWriter(*klass) ||
	resource::IsAxiMasterPort(*klass) ||
	resource::IsAxiSlavePort(*klass)) {
      may_write = true;
    }
    string addr = MemoryAddrPin(res_, 0, accessor);
    string wdata = MemoryWdataPin(res_, 0, accessor);
    if (addr_sel.empty()) {
      addr_sel = addr;
    } else {
      addr_sel = MemoryReqPin(res_, accessor) +
	" ? " + addr + " : (" + addr_sel + ")";
    }
    if (may_write) {
      if (wdata_sel.empty()) {
	wdata_sel = wdata;
      } else {
	wdata_sel = MemoryReqPin(res_, accessor) +
	  " ? " + wdata + " : (" + wdata_sel + ")";
      }
      string wen = "(";
      if (resource::IsSharedMemoryWriter(*klass)) {
	// write only, so assumes req=wen.
	wen += MemoryReqPin(res_, accessor);
      } else {
	wen += MemoryWenPin(res_, 0, accessor);
      }
      wen += " & !" + MemoryAckPin(res_, accessor) + ")";
      if (wen_sel.empty()) {
	wen_sel = wen;
      } else {
	wen_sel = MemoryReqPin(res_, accessor) +
	  " ? " + wen + " : (" + wen_sel + ")";
      }
    }
  }
  ostream &rs = tab_.ResourceSectionStream();
  string addr_wire = MemoryAddrPin(res_, 0, nullptr);
  rs << "  assign " << addr_wire << " = " << addr_sel << ";\n";
  string wdata_wire = MemoryWdataPin(res_, 0, nullptr);
  rs << "  assign " << wdata_wire << " = " << wdata_sel << ";\n";
  string wen_wire = MemoryWenPin(res_, 0, nullptr);
  rs << "  assign " << wen_wire << " = " << wen_sel << ";\n";
}

void SharedMemory::BuildExternalMemoryConnection() {
  IArray *array = res_.GetArray();
  auto *ports = tab_.GetPorts();
  ports->AddPort("sram_addr", Port::OUTPUT_WIRE, array->GetAddressWidth());
  ports->AddPort("sram_wdata", Port::OUTPUT_WIRE, array->GetDataType().GetWidth());
  ports->AddPort("sram_wdata_en", Port::OUTPUT_WIRE, 0);
  ports->AddPort("sram_rdata", Port::INPUT, array->GetAddressWidth());

  ostream &rs = tab_.ResourceSectionStream();
  rs << "  assign sram_addr = " << MemoryAddrPin(res_, 0, nullptr) << ";\n";
  rs << "  assign sram_wdata = " << MemoryWdataPin(res_, 0, nullptr) << ";\n";
  rs << "  assign sram_wdata_en = " << MemoryWenPin(res_, 0, nullptr) << ";\n";
  rs << "  assign " << MemoryRdataPin(res_, 0) << " = sram_rdata;\n";
}

void SharedMemory::BuildMemoryInstance() {
  int num_ports = 1;
  auto &port_users =
    tab_.GetModule()->GetConnection().GetSharedMemoryPort1Accessors(&res_);
  if (port_users.size() > 0) {
    CHECK(port_users.size() == 1);
    num_ports = 2;
  }
  InternalSRAM *sram =
    tab_.GetEmbeddedModules()->RequestInternalSRAM(*tab_.GetModule(),
						   *res_.GetArray(),
						   num_ports);
  auto *ports = tab_.GetPorts();
  ostream &es = tmpl_->GetStream(kEmbeddedInstanceSection);
  string name = sram->GetModuleName();
  string inst = name + "_inst_" + Util::Itoa(tab_.GetITable()->GetId()) +
    "_" + Util::Itoa(res_.GetId());
  string addr_wire = MemoryAddrPin(res_, 0, nullptr);
  string rdata_raw_wire = MemoryRdataRawPin(res_);
  string wdata_wire = MemoryWdataPin(res_, 0, nullptr);
  string wen_wire = MemoryWenPin(res_, 0, nullptr);
  es << "  " << name << " " << inst << "("
     << ".clk(" << ports->GetClk() << ")"
     << ", ." << sram->GetResetPinName() << "(" << ports->GetReset() << ")"
     << ", ." << sram->GetAddrPin(0) << "(" << addr_wire << ")"
     << ", ." << sram->GetRdataPin(0) <<"(" << rdata_raw_wire << ")"
     << ", ." << sram->GetWdataPin(0) <<"(" << wdata_wire << ")"
     << ", ." << sram->GetWenPin(0) <<"(" << wen_wire << ")";
  if (num_ports == 2) {
    es << ", ." << sram->GetAddrPin(1) << "("
       << MemoryAddrPin(res_, 1, nullptr) << ")"
       << ", ." << sram->GetRdataPin(1) <<"("
       << MemoryRdataPin(res_, 1) << ")"
       << ", ." << sram->GetWdataPin(1) <<"("
       << MemoryWdataPin(res_, 1, nullptr) << ")"
       << ", ." << sram->GetWenPin(1) <<"("
       << MemoryWenPin(res_, 1, nullptr) << ")";
  }
  es <<");\n";
  ostream &rs = tab_.ResourceSectionStream();
  rs << "  wire " << sram->AddressWidthSpec() << " " << addr_wire << ";\n";
  rs << "  wire " << sram->DataWidthSpec() << " " << rdata_raw_wire << ";\n";
  rs << "  wire " << sram->DataWidthSpec() << " " << wdata_wire << ";\n";
  rs << "  wire " << wen_wire << ";\n";
  if (num_ports == 2) {
    rs << "  wire " << sram->AddressWidthSpec() << " "
       << MemoryAddrPin(res_, 1, nullptr) << ";\n";
    rs << "  wire " << sram->DataWidthSpec() << " "
       << MemoryRdataPin(res_, 1) << ";\n";
    rs << "  wire " << sram->DataWidthSpec() << " "
       << MemoryWdataPin(res_, 1, nullptr) << ";\n";
    rs << "  wire " << MemoryWenPin(res_, 1, nullptr) << ";\n";
  }
}

void SharedMemory::BuildAccessWireAll(vector<const IResource *> &accessors) {
  IModule *mem_module = res_.GetTable()->GetModule();
  wire::InterModuleWire wire(*this);
  vector<const IResource *> readers;
  for (auto *accessor : accessors) {
    auto *klass = accessor->GetClass();
    bool is_reader = resource::IsSharedMemoryReader(*klass);
    if (is_reader) {
      readers.push_back(accessor);
    }
  }
  string rdata = MemoryRdataPin(res_, 0);
  IArray *array = res_.GetArray();
  int data_width = array->GetDataType().GetWidth();
  // NOTE: rdata can't be shared if there's a distance to the accessor.
  wire.AddSharedWires(readers, rdata, data_width, true, false);
  string rdata_raw = MemoryRdataRawPin(res_);
  ostream &rs = tab_.ResourceSectionStream();
  rs << "  assign " << rdata << " = " << rdata_raw << ";\n";
  for (auto *accessor : accessors) {
    string addr = MemoryAddrPin(res_, 0, accessor);
    int addr_width = array->GetAddressWidth();
    wire.AddWire(*accessor, addr, addr_width, false, true);
    string req = MemoryReqPin(res_, accessor);
    wire.AddWire(*accessor, req, 0, false, true);
    string ack = MemoryAckPin(res_, accessor);
    wire.AddWire(*accessor, ack, 0, true, true);
    auto *klass = accessor->GetClass();
    if (resource::IsSharedMemoryWriter(*klass)) {
      string wdata = MemoryWdataPin(res_, 0, accessor);
      wire.AddWire(*accessor, wdata, 0, false, true);
    }
  }
}

void SharedMemory::BuildMemoryAccessorResource(const Resource &accessor,
					       bool do_write,
					       bool gen_reg,
					       const IResource *mem) {
  bool is_self_mem = resource::IsSharedMemory(*(accessor.GetIResource().GetClass()));
  IArray *array = mem->GetArray();
  int addr_width = array->GetAddressWidth();
  int data_width = array->GetDataType().GetWidth();
  ModuleTemplate *tmpl = accessor.GetModuleTemplate();
  ostream &rs = accessor.GetTable().ResourceSectionStream();
  const IResource &res = accessor.GetIResource();
  string storage;
  if (gen_reg) {
    storage = "reg";
  } else {
    storage = "wire";
  }
  if (!gen_reg) {
    // For an AXI controller.
    rs << "  " << storage << " " << Table::WidthSpec(addr_width)
       << MemoryAddrPin(*mem, 0, &res) << ";\n";
    rs << "  " << storage << " " << MemoryReqPin(*mem, &res) << ";\n";
  }
  const Table &tab = accessor.GetTable();
  ostream &is = tab.InitialValueSectionStream();
  if (gen_reg) {
    is << "      " << MemoryReqPin(*mem, &res) << " <= 0;\n";
  }
  if (do_write && (!gen_reg || is_self_mem)) {
    rs << "  " << storage << " " << Table::WidthSpec(data_width)
       << MemoryWdataPin(*mem, 0, &res) << ";\n";
    rs << "  " << storage << " "
       << MemoryWenPin(*mem, 0, &res) << ";\n";
  }
  // TODO: Output this only for read.
  rs << "  reg " << Table::WidthSpec(data_width) << MemoryRdataBuf(*mem, &res) << ";\n";
  ostream &ss = tab.StateOutputSectionStream();
  map<IState *, IInsn *> callers;
  accessor.CollectResourceCallers("", &callers);
  if (gen_reg) {
    ss << "      " << MemoryReqPin(*mem, &res) << " <= (";
    if (callers.size() > 0) {
      ss << accessor.JoinStatesWithSubState(callers, 0)
	 << ") && !" << MemoryAckPin(*mem, &res)
	 << ";\n";
    } else {
      ss << "0);\n";
    }
  }
  auto *klass = res.GetClass();
  if (resource::IsSharedMemory(*klass)) {
    is << "      " << MemoryWenPin(*mem, 0, &res) << " <= 0;\n";
    map<IState *, IInsn *> writers;
    for (auto it : callers) {
      IInsn *insn = it.second;
      if (insn->inputs_.size() == 2) {
	writers[it.first] = it.second;
      }
    }
    string wen = accessor.JoinStatesWithSubState(writers, 0);
    if (wen.empty()) {
      wen = "0";
    }
    ss << "      " << MemoryWenPin(*mem, 0, &res) << " <= "
       << wen << " && !" << MemoryAckPin(*mem, &res) << ";\n";
  }
}

void SharedMemory::BuildInsn(IInsn *insn, State *st) {
  ostream &os = st->StateBodySectionStream();
  static const char I[] = "          ";
  string st_name = InsnWriter::MultiCycleStateName(*(insn->GetResource()));
  const IResource *mem = nullptr;
  auto *klass = res_.GetClass();
  if (resource::IsSharedMemory(*klass)) {
    mem = &res_;
  } else {
    mem = res_.GetParentResource();
  }
  os << I << "if (" << st_name << " == 0) begin\n";
  os << I << "  " << MemoryAddrPin(*mem, 0, &res_) << " <= "
     << InsnWriter::RegisterValue(*insn->inputs_[0], tab_.GetNames())
     << ";\n";
  if (resource::IsSharedMemoryWriter(*klass) ||
      (resource::IsSharedMemory(*klass) &&
       insn->inputs_.size() == 2)) {
    os << I << "  " << MemoryWdataPin(*mem, 0, &res_) << " <= "
       << InsnWriter::RegisterValue(*insn->inputs_[1], tab_.GetNames())
       << ";\n";
  }
  os << I << "  if (" << MemoryAckPin(*mem, &res_) << ") begin\n"
     << I << "    " << st_name << " <= 3;\n";
  if (resource::IsSharedMemoryReader(*klass) ||
      (resource::IsSharedMemory(*klass) &&
       insn->outputs_.size() == 1)) {
    os << I << "    "
       << MemoryRdataBuf(*mem, &res_)
       << " <= " << MemoryRdataPin(*mem, 0)
       << ";\n";
    ostream &ws = tab_.InsnWireValueSectionStream();
    ws << "  assign " << InsnWriter::InsnOutputWireName(*insn, 0)
       << " = " << MemoryRdataBuf(*mem, &res_) << ";\n";
  }
  os << I << "  end\n";
  os << I << "end\n";
}

string SharedMemory::MemoryPinPrefix(const IResource &mem,
				     const IResource *accessor) {
  string s = "mem_" + Util::Itoa(mem.GetTable()->GetModule()->GetId()) +
    "_" + Util::Itoa(mem.GetTable()->GetId()) +
    "_" + Util::Itoa(mem.GetId());
  if (accessor != nullptr) {
    s += "_" + Util::Itoa(accessor->GetTable()->GetModule()->GetId()) +
      "_" + Util::Itoa(accessor->GetTable()->GetId()) +
      "_" + Util::Itoa(accessor->GetId());
  }
  return s;
}

string SharedMemory::MemoryRdataBuf(const IResource &res, const IResource *accessor) {
  return MemoryPinPrefix(res, accessor) + "_rbuf";
}

string SharedMemory::MemoryAddrPin(const IResource &res,
				   int nth_port,
				   const IResource *accessor) {
  return MemoryPinPrefix(res, accessor) +
    "_p" + Util::Itoa(nth_port) + "_addr";
}

string SharedMemory::MemoryReqPin(const IResource &res,
				  const IResource *accessor) {
  return MemoryPinPrefix(res, accessor) + "_req";
}

string SharedMemory::MemoryAckPin(const IResource &res,
				  const IResource *accessor) {
  return MemoryPinPrefix(res, accessor) + "_ack";
}

string SharedMemory::MemoryRdataPin(const IResource &res, int nth_port) {
  return MemoryPinPrefix(res, nullptr) +
    "_p" + Util::Itoa(nth_port) + "_rdata";
}

string SharedMemory::MemoryRdataRawPin(const IResource &res) {
  return MemoryRdataPin(res, 0) + "_raw";
}

string SharedMemory::MemoryWdataPin(const IResource &res,
				    int nth_port,
				    const IResource *accessor) {
  return MemoryPinPrefix(res, accessor) +
    "_p" + Util::Itoa(nth_port) + "_wdata";
}

string SharedMemory::MemoryWenPin(const IResource &res,
				  int nth_port,
				  const IResource *accessor) {
  return MemoryPinPrefix(res, accessor) +
    "_p" + Util::Itoa(nth_port) + "_wen";
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
