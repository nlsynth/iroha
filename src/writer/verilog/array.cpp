#include "writer/verilog/array.h"

#include "iroha/i_design.h"
#include "iroha/insn_operands.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"
#include "writer/module_template.h"
#include "writer/verilog/embedded_modules.h"
#include "writer/verilog/insn_writer.h"
#include "writer/verilog/internal_sram.h"
#include "writer/verilog/module.h"
#include "writer/verilog/port.h"
#include "writer/verilog/port_set.h"
#include "writer/verilog/state.h"
#include "writer/verilog/table.h"

namespace iroha {
namespace writer {
namespace verilog {

ArrayResource::ArrayResource(const IResource &res, const Table &table)
    : Resource(res, table) {}

void ArrayResource::BuildResource() {
  IArray *array = res_.GetArray();
  if (array->IsExternal()) {
    BuildExternalSRAM();
  } else {
    BuildInternalSRAM();
  }
}

void ArrayResource::BuildInsn(IInsn *insn, State *st) {
  const string &opr = insn->GetOperand();
  if (opr == operand::kSramReadData) {
    ostream &ws = tab_.InsnWireValueSectionStream();
    ws << "  assign " << InsnWriter::InsnOutputWireName(*insn, 0) << " = "
       << SigName("rdata") << ";\n";
  }

  IArray *array = res_.GetArray();
  static const char I[] = "          ";
  ostream &os = st->StateBodySectionStream();
  if (opr == "sram_read_address" || opr == "sram_write") {
    os << I << SigName("addr") << " <= "
       << InsnWriter::RegisterValueWithConstWidth(
              *(insn->inputs_[0]), array->GetAddressWidth(), tab_.GetNames())
       << ";\n";
  }
  if (opr == "sram_write") {
    os << I << SigName("wdata") << " <= "
       << InsnWriter::RegisterValueWithConstWidth(
              *(insn->inputs_[1]), array->GetDataType().GetWidth(),
              tab_.GetNames())
       << ";\n";
  }
}

void ArrayResource::BuildExternalSRAM() {
  IArray *array = res_.GetArray();
  int data_width = array->GetDataType().GetWidth();
  AddPortToTop(SigName("addr"), true, false, array->GetAddressWidth());
  AddPortToTop(SigName("rdata"), false, false, data_width);
  AddPortToTop(SigName("wdata"), true, false, data_width);
  AddPortToTop(SigName("wdata_en"), true, false, 0);
  BuildSRAMWrite();
}

void ArrayResource::BuildInternalSRAM() {
  InternalSRAM *sram = tab_.GetEmbeddedModules()->RequestInternalSRAM(
      *tab_.GetModule(), *res_.GetArray(), 1);
  auto *ports = tab_.GetPortSet();
  ostream &es = tmpl_->GetStream(kEmbeddedInstanceSection);
  string name = sram->GetModuleName();
  string inst = name + "_inst_" + Util::Itoa(res_.GetId());
  es << "  " << name << " " << inst << "("
     << ".clk(" << ports->GetClk() << ")"
     << ", ." << sram->GetResetPinName() << "(" << ports->GetReset() << ")"
     << ", ." << sram->GetAddrPin(0) << "(" << SigName("addr") << ")"
     << ", ." << sram->GetRdataPin(0) << "(" << SigName("rdata") << ")"
     << ", ." << sram->GetWdataPin(0) << "(" << SigName("wdata") << ")"
     << ", ." << sram->GetWenPin(0) << "(" << SigName("wdata_en") << ")"
     << ");\n";
  ostream &rs = tab_.ResourceSectionStream();
  rs << "  reg " << sram->AddressWidthSpec() << SigName("addr") << ";\n"
     << "  wire " << sram->DataWidthSpec() << SigName("rdata") << ";\n"
     << "  reg " << sram->DataWidthSpec() << SigName("wdata") << ";\n"
     << "  reg " << SigName("wdata_en") << ";\n";
  BuildSRAMWrite();
}

void ArrayResource::BuildSRAMWrite() {
  map<IState *, IInsn *> callers;
  CollectResourceCallers("sram_write", &callers);
  ostream &fs = tab_.StateOutputSectionStream();
  fs << "      " << SigName("wdata_en") << " <= ";
  WriteStateUnion(callers, fs);
  fs << ";\n";
}

string ArrayResource::SigName(const string &sig) { return SigName(res_, sig); }

string ArrayResource::SigName(const IResource &res, const string &sig) {
  IArray *array = res.GetArray();
  string res_id;
  if (array != nullptr && array->IsExternal()) {
    string prefix = res.GetParams()->GetPortNamePrefix();
    if (!prefix.empty()) {
      res_id = "_" + prefix;
    }
  } else {
    res_id = Util::Itoa(res.GetId());
  }
  return "sram" + res_id + "_" + sig;
}

IArray *ArrayResource::GetArray() { return res_.GetArray(); }

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
