#include "writer/verilog/embed.h"

#include "iroha/i_design.h"
#include "iroha/logging.h"
#include "iroha/resource_params.h"
#include "writer/module_template.h"
#include "writer/verilog/insn_writer.h"
#include "writer/verilog/internal_sram.h"
#include "writer/verilog/module.h"
#include "writer/verilog/ports.h"
#include "writer/verilog/state.h"
#include "writer/verilog/table.h"

namespace iroha {
namespace writer {
namespace verilog {

EmbeddedModules::~EmbeddedModules() {
}

void EmbeddedModules::RequestModule(const ResourceParams &params) {
  string name = params.GetEmbeddedModuleFileName();
  if (!name.empty()) {
    files_.insert(name);
  }
}

bool EmbeddedModules::Write(ostream &os) {
  // Files
  for (auto &s : files_) {
    istream *ifs = Util::OpenFile(s);
    if (ifs == nullptr) {
      LOG(ERROR) << "Failed to open: " << s;
      return false;
    }
    unique_ptr<istream> deleter(ifs);
    os << "// Copied from " << s << "\n";
    while (!ifs->eof()) {
      string line;
      getline(*ifs, line);
      os << line << "\n";
    }
    os << "//\n";
  }
  if (files_.size() > 0) {
    os << "\n";
  }
  // Internal SRAM
  for (InternalSRAM *sram : srams_) {
    sram->Write(os);
  }
  return true;
}

InternalSRAM *EmbeddedModules::RequestInternalSRAM(const Module &mod,
						   const IResource &res) {
  InternalSRAM *sram = new InternalSRAM(mod, res);
  srams_.push_back(sram);
  return sram;
}

EmbeddedResource::EmbeddedResource(const IResource &res, const Table &table)
  : Resource(res, table) {
}

void EmbeddedResource::BuildResource() {
  auto *embed = tab_.GetEmbeddedModules();
  auto *params = res_.GetParams();
  embed->RequestModule(*params);

  ostream &is = tmpl_->GetStream(kEmbeddedInstanceSection);
  string name = params->GetEmbeddedModuleName();
  auto *ports = tab_.GetPorts();
  is << "  // " << name << "\n";
  is << "  " << name << " inst_" << name << "(";
  is << "." << params->GetEmbeddedModuleClk() << "(" << ports->GetClk() << "), "
     << "." << params->GetEmbeddedModuleReset() << "(" << ports->GetReset() << ")";
  ostream &rs = tab_.InitialValueSectionStream();
  ostream &ws = tmpl_->GetStream(kInsnWireDeclSection);
  string ack = params->GetEmbeddedModuleAck();
  if (!ack.empty()) {
    string ack_wire = "ack_" + name;
    ws << "  wire " << ack_wire << ";\n";
    is << ", ." << ack << "(" << ack_wire << ")";
  }
  string req = params->GetEmbeddedModuleReq();
  if (!req.empty()) {
    string req_reg = "req_" + name;
    ws << "  reg " << req_reg << ";\n";
    rs << "      " << req_reg << " <= 0;\n";
    is << ", ." << req << "(" << req_reg << ")";
  }
  is << ");\n";
}

void EmbeddedResource::BuildInsn(IInsn *insn, State *st) {
  static const char I[] = "          ";
  string insn_st = InsnWriter::MultiCycleStateName(*(insn->GetResource()));
  ostream &os = st->StateBodySectionStream();
  os << I << "// embedded module\n"
     << I << "if (" << insn_st << " == 0) begin\n"
     << I << "  " << insn_st << " <= 1;\n";
  auto *params = res_.GetParams();
  string name = params->GetEmbeddedModuleName();
  string req = params->GetEmbeddedModuleReq();
  if (!req.empty()) {
    os << I << "  req_" << name << " <= 1;\n";
  }
  os << I << "end\n";
  string ack = params->GetEmbeddedModuleAck();
  os << I << "if (" << insn_st << " == 1) begin\n";
  if (!ack.empty()) {
    os << I << "if (ack_" << name << " == 1) begin\n";
    os << I << "  req_" << name << " <= 0;\n";
  }
  os << I << "  " << insn_st << " <= 3;\n";
  if (!ack.empty()) {
    os << I << "end\n";
  }
  os << I << "end\n";
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
