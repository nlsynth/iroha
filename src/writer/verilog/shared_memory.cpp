#include "writer/verilog/shared_memory.h"

#include "iroha/i_design.h"
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
    BuildMemoryAccessorResource(true);
  }
  if (resource::IsSharedMemoryReader(*klass)) {
    BuildMemoryAccessorResource(false);
  }
  if (resource::IsSharedMemoryWriter(*klass)) {
    BuildMemoryAccessorResource(true);
  }
}

void SharedMemory::BuildMemoryResource() {
  BuildMemoryInstance();
  vector<const IResource *> accessors;
  accessors.push_back(&res_);
  auto *ext_accessors =
    tab_.GetModule()->GetConnection().GetSharedMemoryAccessors(&res_);
  for (IResource *r : *ext_accessors) {
    accessors.push_back(r);
  }
  BuildAccessWireAll(accessors);
  ostream &is = tab_.InitialValueSectionStream();
  ostream &ss = tab_.StateOutputSectionStream();
  ostream &rs = tmpl_->GetStream(kResourceSection);
  string high_req = "0";
  for (auto *accessor : accessors) {
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
      string wen;
      if (resource::IsSharedMemory(*klass)) {
	wen = MemoryWenPin(res_, accessor);
      } else {
	wen = MemoryReqPin(res_, accessor);
      }
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
     << ", ." << sram->GetAddrPin(0) << "(" << addr_wire << ")"
     << ", ." << sram->GetRdataPin(0) <<"(" << rdata_wire << ")"
     << ", ." << sram->GetWdataPin(0) <<"(" << wdata_wire << ")"
     << ", ." << sram->GetWenPin(0) <<"(" << wen_wire << ")"
     <<");\n";
  ostream &rs = tmpl_->GetStream(kResourceSection);
  rs << "  wire " << sram->AddressWidthSpec() << " " << addr_wire << ";\n";
  rs << "  wire " << sram->DataWidthSpec() << " " << rdata_wire << ";\n";
  rs << "  wire " << sram->DataWidthSpec() << " " << wdata_wire << ";\n";
  rs << "  wire " << wen_wire << ";\n";
}

void SharedMemory::BuildAccessWireAll(vector<const IResource *> &accessors) {
  IModule *mem_module = res_.GetTable()->GetModule();
  for (auto *accessor : accessors) {
    IModule *accessor_module = accessor->GetTable()->GetModule();
    const IModule *common_root = Connection::GetCommonRoot(mem_module,
							   accessor_module);
    auto *klass = accessor->GetClass();
    bool is_reader = resource::IsSharedMemoryReader(*klass);
    if (accessor_module != common_root) {
      AddWire(common_root, accessor);
      if (is_reader && mem_module != common_root) {
	AddRdataWire(common_root, accessor);
      }
    }
    // upward
    for (IModule *imod = accessor_module; imod != common_root;
	 imod = imod->GetParentModule()) {
      AddAccessPort(imod, accessor, true);
      if (is_reader) {
	AddRdataPort(imod, accessor, true);
      }
    }
    // downward
    for (IModule *imod = mem_module; imod != common_root;
	 imod = imod->GetParentModule()) {
      AddAccessPort(imod, accessor, false);
      if (is_reader) {
	AddRdataPort(imod, accessor, false);
      }
    }
  }
}

void SharedMemory::BuildMemoryAccessorResource(bool is_writer) {
  const IResource *mem;
  auto *klass = res_.GetClass();
  if (resource::IsSharedMemory(*klass)) {
    mem = &res_;
  } else {
    mem = res_.GetSharedRegister();
  }
  IArray *array = mem->GetArray();
  int addr_width = array->GetAddressWidth();
  ostream &rs = tmpl_->GetStream(kResourceSection);
  rs << "  reg " << Table::WidthSpec(addr_width)
     << MemoryAddrPin(*mem, &res_) << ";\n";
  rs << "  reg " << MemoryReqPin(*mem, &res_) << ";\n";
  ostream &is = tab_.InitialValueSectionStream();
  is << "      " << MemoryReqPin(*mem, &res_) << " <= 0;\n";
  if (is_writer) {
    int data_width = array->GetDataType().GetWidth();
    rs << "  reg " << Table::WidthSpec(data_width)
     << MemoryWdataPin(*mem, &res_) << ";\n";
  }
  ostream &ss = tab_.StateOutputSectionStream();
  map<IState *, IInsn *> callers;
  CollectResourceCallers("", &callers);
  ss << "      " << MemoryReqPin(*mem, &res_) << " <= "
     << JoinStatesWithSubState(callers, 0) << ";\n";
  if (resource::IsSharedMemory(*klass)) {
    rs << "  reg " << MemoryWenPin(*mem, &res_) << ";\n";
    is << "      " << MemoryWenPin(*mem, &res_) << " <= 0;\n";
    map<IState *, IInsn *> writers;
    for (auto it : callers) {
      IInsn *insn = it.second;
      if (insn->inputs_.size() == 2) {
	writers[it.first] = it.second;
      }
    }
      ss << "      " << MemoryWenPin(*mem, &res_) << " <= "
	 << JoinStatesWithSubState(writers, 0) << ";\n";
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
    mem = res_.GetSharedRegister();
  }
  os << I << "if (" << st_name << " == 0) begin\n";
  os << I << "  " << MemoryAddrPin(*mem, &res_) << " <= "
     << InsnWriter::RegisterValue(*insn->inputs_[0], tab_.GetNames())
     << ";\n";
  if (resource::IsSharedMemoryWriter(*klass) ||
      (resource::IsSharedMemory(*klass) &&
       insn->inputs_.size() == 2)) {
    os << I << "  " << MemoryWdataPin(*mem, &res_) << " <= "
       << InsnWriter::RegisterValue(*insn->inputs_[1], tab_.GetNames())
       << ";\n";
  }
  os << I << "  if (" << MemoryAckPin(*mem, &res_) << ") begin\n"
     << I << "    " << st_name << " <= 3;\n";
  if (resource::IsSharedMemoryReader(*klass) ||
      (resource::IsSharedMemory(*klass) &&
       insn->outputs_.size() == 1)) {
    os << I << "    "
       << InsnWriter::RegisterValue(*insn->outputs_[0], tab_.GetNames())
       << " <= " << MemoryRdataPin(*mem)
       << ";\n";
  }
  os << I << "  end\n";
  os << I << "end\n";
}

void SharedMemory::AddAccessPort(const IModule *imod,
				 const IResource *accessor, bool upward) {
  Module *mod = tab_.GetModule()->GetByIModule(imod);
  Ports *ports = mod->GetPorts();
  IArray *array = res_.GetArray();
  int addr_width = array->GetAddressWidth();
  int data_width = array->GetDataType().GetWidth();
  if (upward) {
    ports->AddPort(MemoryAddrPin(res_, accessor), Port::OUTPUT_WIRE, addr_width);
    ports->AddPort(MemoryReqPin(res_, accessor), Port::OUTPUT_WIRE, 0);
    ports->AddPort(MemoryAckPin(res_, accessor), Port::INPUT, 0);
  } else {
    ports->AddPort(MemoryAddrPin(res_, accessor), Port::INPUT, addr_width);
    ports->AddPort(MemoryReqPin(res_, accessor), Port::INPUT, 0);
    ports->AddPort(MemoryAckPin(res_, accessor), Port::OUTPUT_WIRE, 0);
  }
  auto *klass = accessor->GetClass();
  if (resource::IsSharedMemoryWriter(*klass)) {
    if (upward) {
      ports->AddPort(MemoryWdataPin(res_, accessor), Port::OUTPUT_WIRE,
		    data_width);
    } else {
      ports->AddPort(MemoryWdataPin(res_, accessor), Port::INPUT,
		    data_width);
    }
  }
}

void SharedMemory::AddRdataPort(const IModule *imod, const IResource *accessor,
				bool upward) {
  Module *mod = tab_.GetModule()->GetByIModule(imod);
  Ports *ports = mod->GetPorts();
  IArray *array = res_.GetArray();
  int data_width = array->GetDataType().GetWidth();
  if (upward) {
    ports->AddPort(MemoryRdataPin(res_), Port::INPUT,
		   data_width);
  } else {
    ports->AddPort(MemoryRdataPin(res_), Port::OUTPUT_WIRE,
		   data_width);
  }
}

void SharedMemory::AddWire(const IModule *imod, const IResource *accessor) {
  Module *mod = tab_.GetModule()->GetByIModule(imod);
  auto *tmpl = mod->GetModuleTemplate();
  ostream &rs = tmpl->GetStream(kResourceSection);
  IArray *array = res_.GetArray();
  int addr_width = array->GetAddressWidth();
  auto *klass = accessor->GetClass();
  rs << "  wire " << Table::WidthSpec(addr_width);
  rs << MemoryAddrPin(res_, accessor) << ";\n";
  rs << "  wire " << MemoryReqPin(res_, accessor) << ";\n";
  if (resource::IsSharedMemoryWriter(*klass)) {
    int data_width = array->GetDataType().GetWidth();
    rs << "  wire " << Table::WidthSpec(data_width);
    rs << MemoryWdataPin(res_, accessor) << ";\n";
  }
}

void SharedMemory::AddRdataWire(const IModule *imod, const IResource *accessor) {
  Module *mod = tab_.GetModule()->GetByIModule(imod);
  auto *tmpl = mod->GetModuleTemplate();
  ostream &rs = tmpl->GetStream(kResourceSection);
  IArray *array = res_.GetArray();
  int data_width = array->GetDataType().GetWidth();
  rs << "  wire " << Table::WidthSpec(data_width);
  rs << MemoryRdataPin(res_) << ";\n";
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
