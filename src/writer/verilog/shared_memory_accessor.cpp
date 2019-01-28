#include "writer/verilog/shared_memory_accessor.h"

#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "writer/verilog/insn_writer.h"
#include "writer/verilog/shared_memory.h"
#include "writer/verilog/state.h"
#include "writer/verilog/table.h"

namespace iroha {
namespace writer {
namespace verilog {

SharedMemoryAccessor::SharedMemoryAccessor(const IResource &res,
					   const Table &table)
  : Resource(res, table) {
}

void SharedMemoryAccessor::BuildResource() {
  auto *klass = res_.GetClass();
  if (resource::IsSharedMemoryReader(*klass)) {
    BuildMemoryAccessorResource(*this, false, true, res_.GetParentResource());
  }
  if (resource::IsSharedMemoryWriter(*klass)) {
    BuildMemoryAccessorResource(*this, true, true, res_.GetParentResource());
  }
}

void SharedMemoryAccessor::BuildInsn(IInsn *insn, State *st) {
  BuildAccessInsn(insn, st, res_, tab_);
}

void SharedMemoryAccessor::BuildMemoryAccessorResource(const Resource &accessor,
						       bool do_write,
						       bool gen_reg,
						       const IResource *mem) {
  bool is_self_mem =
    resource::IsSharedMemory(*(accessor.GetIResource().GetClass()));
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
       << SharedMemory::MemoryAddrPin(*mem, 0, &res) << ";\n";
    rs << "  " << storage << " " << SharedMemory::MemoryReqPin(*mem, &res)
       << ";\n";
  }
  const Table &tab = accessor.GetTable();
  ostream &is = tab.InitialValueSectionStream();
  if (gen_reg) {
    is << "      " << SharedMemory::MemoryReqPin(*mem, &res) << " <= 0;\n";
  }
  if (do_write && (!gen_reg || is_self_mem)) {
    rs << "  " << storage << " " << Table::WidthSpec(data_width)
       << SharedMemory::MemoryWdataPin(*mem, 0, &res) << ";\n";
    rs << "  " << storage << " "
       << SharedMemory::MemoryWenPin(*mem, 0, &res) << ";\n";
  }
  // TODO: Output this only for read.
  rs << "  reg " << Table::WidthSpec(data_width)
     << SharedMemory::MemoryRdataBuf(*mem, &res) << ";\n";
  ostream &ss = tab.StateOutputSectionStream();
  map<IState *, IInsn *> callers;
  accessor.CollectResourceCallers("", &callers);
  if (gen_reg) {
    ss << "      " << SharedMemory::MemoryReqPin(*mem, &res) << " <= (";
    if (callers.size() > 0) {
      ss << accessor.JoinStatesWithSubState(callers, 0)
	 << ") && !" << SharedMemory::MemoryAckPin(*mem, &res)
	 << ";\n";
    } else {
      ss << "0);\n";
    }
  }
  auto *klass = res.GetClass();
  if (resource::IsSharedMemory(*klass)) {
    is << "      " << SharedMemory::MemoryWenPin(*mem, 0, &res) << " <= 0;\n";
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
    ss << "      " << SharedMemory::MemoryWenPin(*mem, 0, &res) << " <= "
       << wen << " && !" << SharedMemory::MemoryAckPin(*mem, &res) << ";\n";
  }
}

void SharedMemoryAccessor::BuildAccessInsn(IInsn *insn, State *st,
					   const IResource &res,
					   const Table &tab) {
  ostream &os = st->StateBodySectionStream();
  static const char I[] = "          ";
  string st_name = InsnWriter::MultiCycleStateName(*(insn->GetResource()));
  const IResource *mem = nullptr;
  auto *klass = res.GetClass();
  if (resource::IsSharedMemory(*klass)) {
    mem = &res;
  } else {
    mem = res.GetParentResource();
  }
  os << I << "if (" << st_name << " == 0) begin\n";
  os << I << "  " << SharedMemory::MemoryAddrPin(*mem, 0, &res) << " <= "
     << InsnWriter::RegisterValue(*insn->inputs_[0], tab.GetNames())
     << ";\n";
  if (resource::IsSharedMemoryWriter(*klass) ||
      (resource::IsSharedMemory(*klass) &&
       insn->inputs_.size() == 2)) {
    os << I << "  " << SharedMemory::MemoryWdataPin(*mem, 0, &res) << " <= "
       << InsnWriter::RegisterValue(*insn->inputs_[1], tab.GetNames())
       << ";\n";
  }
  os << I << "  if (" << SharedMemory::MemoryAckPin(*mem, &res) << ") begin\n"
     << I << "    " << st_name << " <= 3;\n";
  if (resource::IsSharedMemoryReader(*klass) ||
      (resource::IsSharedMemory(*klass) &&
       insn->outputs_.size() == 1)) {
    os << I << "    "
       << SharedMemory::MemoryRdataBuf(*mem, &res)
       << " <= " << SharedMemory::MemoryRdataPin(*mem, 0)
       << ";\n";
    ostream &ws = tab.InsnWireValueSectionStream();
    ws << "  assign " << InsnWriter::InsnOutputWireName(*insn, 0)
       << " = " << SharedMemory::MemoryRdataBuf(*mem, &res) << ";\n";
  }
  os << I << "  end\n";
  os << I << "end\n";
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
