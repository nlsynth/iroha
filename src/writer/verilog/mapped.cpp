#include "writer/verilog/mapped.h"

#include "iroha/i_design.h"
#include "iroha/resource_params.h"
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

MappedResource::MappedResource(const IResource &res, const Table &table)
  : Resource(res, table) {
}

void MappedResource::BuildResource() {
  auto *params = res_.GetParams();
  if (params->GetMappedName() == "mem") {
    IArray *array = res_.GetArray();
    if (array->IsExternal()) {
      BuildExternalSRAM();
    } else {
      BuildInternalSRAM();
    }
  }
}

void MappedResource::BuildInsn(IInsn *insn, State *st) {
  auto *params = res_.GetParams();
  if (params->GetMappedName() == "mem") {
    BuildMemInsn(insn, st);
  }
}

void MappedResource::BuildMemInsn(IInsn *insn, State *st) {
  string res_id;
  IArray *array = res_.GetArray();
  if (!array->IsExternal()) {
    res_id = "_" + Util::Itoa(res_.GetId());
  }
  const string &opr = insn->GetOperand();
  if (opr == "sram_read_data") {
    ostream &ws = tmpl_->GetStream(kInsnWireValueSection);
    ws << "  assign " << InsnWriter::InsnOutputWireName(*insn, 0)
       << " = sram_rdata" << res_id << ";\n";
  }

  static const char I[] = "          ";
  ostream &os = st->StateBodySectionStream();
  if (opr == "sram_read_address" ||
      opr == "sram_write") {
    os << I << "sram_addr" + res_id << " <= "
       << InsnWriter::RegisterName(*(insn->inputs_[0]))
       << ";\n";
  }
  if (opr == "sram_write") {
    os << I << "sram_wdata" + res_id << " <= "
       << InsnWriter::RegisterName(*(insn->inputs_[1]))
       << ";\n";
  }
}

void MappedResource::BuildExternalSRAM() {
  auto *ports = tab_.GetPorts();
  IArray *array = res_.GetArray();
  int data_width = array->GetDataType().GetWidth();
  ports->AddPort("sram_addr", Port::OUTPUT, array->GetAddressWidth());
  ports->AddPort("sram_rdata", Port::INPUT, data_width);
  ports->AddPort("sram_wdata", Port::OUTPUT, data_width);
  ports->AddPort("sram_wdata_en", Port::OUTPUT, 0);
  BuildSRAMWrite("");
}

void MappedResource::BuildInternalSRAM() {
  InternalSRAM *sram =
    tab_.GetEmbeddedModules()->RequestInternalSRAM(*tab_.GetModule(), res_);
  auto *ports = tab_.GetPorts();
  ostream &es = tmpl_->GetStream(kEmbeddedInstanceSection);
  string name = sram->GetModuleName();
  string res_id = Util::Itoa(res_.GetId());
  string inst = name + "_inst_" + res_id;
  es << "  " << name << " " << inst << "("
     << ".clk(" << ports->GetClk() << ")"
     << ", ." << sram->GetResetPinName() << "(" << ports->GetReset() << ")"
     << ", .addr_i(sram_addr_" << res_id << ")"
     << ", .rdata_o(sram_rdata_" << res_id << ")"
     << ", .wdata_i(sram_wdata_" << res_id << ")"
     << ", .write_en_i(sram_wdata_en_" << res_id << ")"
     <<");\n";
  ostream &rs = tmpl_->GetStream(kResourceSection);
  rs << "  reg " << sram->AddressWidthSpec() << "sram_addr_" << res_id << ";\n"
     << "  wire " << sram->DataWidthSpec() << "sram_rdata_" << res_id << ";\n"
     << "  reg " << sram->DataWidthSpec() << "sram_wdata_" << res_id << ";\n"
     << "  reg sram_wdata_en_" << res_id << ";\n";
  BuildSRAMWrite("_" + res_id);
}

void MappedResource::BuildSRAMWrite(const string &res_id) {
  map<IState *, IInsn *> callers;
  CollectResourceCallers("sram_write", &callers);
  ostream &fs = tab_.StateOutputSectionStream();
  fs << "      sram_wdata_en" << res_id << " <= ";
  WriteStateUnion(callers, fs);
  fs << ";\n";
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
