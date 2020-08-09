#include "writer/verilog/sram_if.h"

#include "iroha/i_design.h"
#include "iroha/resource_params.h"
#include "writer/verilog/insn_writer.h"
#include "writer/verilog/module.h"
#include "writer/verilog/shared_memory.h"
#include "writer/verilog/state.h"
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
  os << "  assign " << SharedMemory::MemoryAddrPin(*mem, 1, nullptr)
     << " = " << prefix << "addr;\n";
  os << "  assign " << SharedMemory::MemoryWenPin(*mem, 1, nullptr)
     << " = " << prefix << "wen;\n";
  os << "  assign " << SharedMemory::MemoryWdataPin(*mem, 1, nullptr)
     << " = " << prefix << "wdata;\n";
  os << "  assign " << prefix << "rdata = "
     << SharedMemory::MemoryRdataPin(*mem, 1) << ";\n";

  map<IState *, IInsn *> callers;
  CollectResourceCallers("*", &callers);
  if (callers.size() > 0) {
    ostream &rs = tab_.ResourceSectionStream();
    rs << "  reg " << AckReg() << ";\n"
       << "  reg " << NotifyReg() << ";\n";

    ostream &is = tab_.InitialValueSectionStream();
    is << "      " << AckReg() << " <= 0;\n"
       << "      " << NotifyReg() << " <= 0;\n";

    ostream &ss = tab_.StateOutputSectionStream();
    ss << "      " << NotifyReg() << " <= (" << NotifyReg() << "|"
       << (prefix + "wen") << ") & ~" << AckReg() << ";\n";
  }
}

void SramIf::BuildInsn(IInsn *insn, State *st) {
  static const char I[] = "          ";
  string insn_st = InsnWriter::MultiCycleStateName(*(insn->GetResource()));
  string prefix = res_.GetParams()->GetPortNamePrefix();
  ostream &os = st->StateBodySectionStream();
  os << I << "// SRAM notification wait\n"
     << I << "if (" << insn_st << " == 0) begin\n"
     << I << "  if (" << (prefix + "wen") << ") begin\n"
     << I << "    " << insn_st << " <= 1;\n"
     << I << "    " << AckReg() << " <= 1;\n"
     << I << "  end\n"
     << I << "end\n"
     << I << "if (" << insn_st << " == 1) begin\n"
     << I << "  " << AckReg() << " <= 0;\n"
     << I << "  " << insn_st << " <= 3;\n"
     << I << "end\n";
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

string SramIf::AckReg() {
  string prefix = res_.GetParams()->GetPortNamePrefix();
  return prefix + "ack";
}

string SramIf::NotifyReg() {
  string prefix = res_.GetParams()->GetPortNamePrefix();
  return prefix + "notify";
}

string SramIf::WenPort(IResource *res) {
  string prefix = res->GetParams()->GetPortNamePrefix();
  return prefix + "wen";
}


}  // namespace verilog
}  // namespace writer
}  // namespace iroha
