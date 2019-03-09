#include "writer/verilog/shared_memory_accessor.h"

#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "writer/verilog/insn_writer.h"
#include "writer/verilog/shared_memory.h"
#include "writer/verilog/state.h"
#include "writer/verilog/table.h"
#include "writer/verilog/wire/wire_set.h"

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
    // driven from axi controller.
    storage = "wire";
  }
  string rn = SharedMemory::GetName(*mem);
  // Addr.
  rs << "  " << storage << " " << Table::WidthSpec(addr_width)
     << AddrSrc(res) << ";\n";
  rs << "  assign " << wire::Names::AccessorWire(rn, &res, "addr")
     << " = " << AddrSrc(res) << ";\n";
  // Req.
  string req = ReqSrc(res);
  rs << "  " << storage << " " << req << ";\n";
  rs << "  assign " << wire::Names::AccessorWire(rn, &res, "req")
     << " = " << req << ";\n";
  const Table &tab = accessor.GetTable();
  ostream &is = tab.InitialValueSectionStream();
  if (gen_reg) {
    is << "      " << req << " <= 0;\n";
  }
  // WData and WEn.
  if (do_write) {
    string wdata = WDataSrc(res);
    rs << "  " << storage << " " << Table::WidthSpec(data_width)
       << wdata << ";\n";
    rs << "  assign " << wire::Names::AccessorWire(rn, &res, "wdata")
       << " = " << wdata << ";\n";
    string wen = WEnSrc(res);
    rs << "  " << storage << " " << wen << ";\n";
    rs << "  assign " << wire::Names::AccessorWire(rn, &res, "wen")
       << " = " << wen << ";\n";
  }

  auto *klass = res.GetClass();
  if (!resource::IsSharedMemoryWriter(*klass)) {
    rs << "  reg " << Table::WidthSpec(data_width)
       << SharedMemory::MemoryRdataBuf(*mem, &res) << ";\n";
  }
  ostream &ss = tab.StateOutputSectionStream();
  map<IState *, IInsn *> callers;
  accessor.CollectResourceCallers("", &callers);
  string ack = wire::Names::AccessorWire(rn, &res, "ack");
  if (gen_reg) {
    ss << "      " << ReqSrc(res) << " <= (";
    if (callers.size() > 0) {
      ss << accessor.JoinStatesWithSubState(callers, 0)
	 << ") && !" << ack
	 << ";\n";
    } else {
      ss << "0);\n";
    }
  }

  if (resource::IsSharedMemory(*klass) ||
      resource::IsSharedMemoryWriter(*klass)) {
    is << "      " << WEnSrc(res) << " <= 0;\n";
    map<IState *, IInsn *> writers;
    if (resource::IsSharedMemory(*klass)) {
      for (auto it : callers) {
	IInsn *insn = it.second;
	if (insn->inputs_.size() == 2) {
	  writers[it.first] = it.second;
	}
      }
    } else {
      writers = callers;
    }
    string wen = accessor.JoinStatesWithSubState(writers, 0);
    if (wen.empty()) {
      wen = "0";
    }
    ss << "      " << WEnSrc(res) << " <= "
       << wen << " && !" << ack << ";\n";
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
  os << I << "  " << AddrSrc(res) << " <= "
     << InsnWriter::RegisterValue(*insn->inputs_[0], tab.GetNames())
     << ";\n";
  if (resource::IsSharedMemoryWriter(*klass) ||
      (resource::IsSharedMemory(*klass) &&
       insn->inputs_.size() == 2)) {
    os << I << "  " << WDataSrc(res) << " <= "
       << InsnWriter::RegisterValue(*insn->inputs_[1], tab.GetNames())
       << ";\n";
  }
  string rn = SharedMemory::GetName(*mem);
  string ack = wire::Names::AccessorWire(rn, &res, "ack");
  string rdata = wire::Names::AccessorWire(rn, &res, "rdata");
  os << I << "  if (" << ack << ") begin\n"
     << I << "    " << st_name << " <= 3;\n";
  if (resource::IsSharedMemoryReader(*klass) ||
      (resource::IsSharedMemory(*klass) &&
       insn->outputs_.size() == 1)) {
    os << I << "    "
       << SharedMemory::MemoryRdataBuf(*mem, &res)
       << " <= " << rdata
       << ";\n";
    ostream &ws = tab.InsnWireValueSectionStream();
    ws << "  assign " << InsnWriter::InsnOutputWireName(*insn, 0)
       << " = " << SharedMemory::MemoryRdataBuf(*mem, &res) << ";\n";
  }
  os << I << "  end\n";
  os << I << "end\n";
}

string SharedMemoryAccessor::AddrSrc(const IResource &res) {
  return SrcName(res, "addr");
}

string SharedMemoryAccessor::ReqSrc(const IResource &res) {
  return SrcName(res, "req");
}

string SharedMemoryAccessor::WDataSrc(const IResource &res) {
  return SrcName(res, "wdata");
}

string SharedMemoryAccessor::WEnSrc(const IResource &res) {
  return SrcName(res, "wen");
}

const IResource *SharedMemoryAccessor::GetMem(const IResource &accessor) {
  const auto *mem = accessor.GetParentResource();
  if (mem == nullptr) {
    return &accessor;
  }
  return mem;
}

string SharedMemoryAccessor::SrcName(const IResource &accessor,
				     const string &name) {
  const auto *mem = GetMem(accessor);
  string rn = SharedMemory::GetName(*mem);
  return wire::Names::AccessorSignalBase(rn, &accessor, name.c_str()) + "_src";
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
