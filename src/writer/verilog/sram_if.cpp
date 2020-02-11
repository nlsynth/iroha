#include "writer/verilog/sram_if.h"

#include "iroha/i_design.h"
#include "iroha/resource_params.h"
#include "writer/verilog/module.h"
#include "writer/verilog/shared_memory.h"
#include "writer/verilog/table.h"

namespace iroha {
namespace writer {
namespace verilog {

SramIf::SramIf(const IResource &res, const Table &table)
  : Resource(res, table) {
}

void SramIf::BuildResource() {
  Module *mod = tab_.GetModule();
  AddPorts(mod);
  ostream &os = tab_.ResourceSectionStream();
  IResource *mem = res_.GetParentResource();
  string prefix = res_.GetParams()->GetPortNamePrefix();
  os << "  assign " << SharedMemory::MemoryAddrPin(*mem, 1, nullptr) << " = " << prefix << "addr;\n";
  os << "  assign " << SharedMemory::MemoryWenPin(*mem, 1, nullptr) << " = " << prefix << "wen;\n";
  os << "  assign " << SharedMemory::MemoryWdataPin(*mem, 1, nullptr) << " = " << prefix << "wdata;\n";
  os << "  assign " << prefix << "rdata = " << SharedMemory::MemoryRdataPin(*mem, 1) << ";\n";
}

void SramIf::BuildInsn(IInsn *insn, State *st) {
}

void SramIf::CollectNames(Names *names) {
}

void SramIf::AddPorts(Module *mod) {
  string prefix = res_.GetParams()->GetPortNamePrefix();
  IArray *array = res_.GetParentResource()->GetArray();
  int aw = array->GetAddressWidth();

  int dw = array->GetDataType().GetWidth();
  AddPortToTop(prefix + "addr", false, true, aw);
  AddPortToTop(prefix + "wen", false, true, 0);
  AddPortToTop(prefix + "rdata", true, true, dw);
  AddPortToTop(prefix + "wdata", false, true, dw);
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
