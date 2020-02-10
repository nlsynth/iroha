#include "writer/verilog/sram_if.h"

#include "iroha/i_design.h"
#include "iroha/resource_params.h"
#include "writer/verilog/module.h"
#include "writer/verilog/ports.h"
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
  Ports *ports = mod->GetPorts();
  IArray *array = res_.GetParentResource()->GetArray();
  int aw = array->GetAddressWidth();

  int dw = array->GetDataType().GetWidth();
  ports->AddPrefixedPort(prefix, "addr", Port::INPUT, aw);
  ports->AddPrefixedPort(prefix, "wen", Port::INPUT, 0);
  ports->AddPrefixedPort(prefix, "rdata", Port::OUTPUT_WIRE, dw);
  ports->AddPrefixedPort(prefix, "wdata", Port::INPUT, dw);
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
