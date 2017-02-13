#include "writer/verilog/embed.h"

#include "iroha/i_design.h"
#include "iroha/logging.h"
#include "iroha/resource_params.h"
#include "writer/module_template.h"
#include "writer/verilog/axi/axi_port.h"
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

void EmbeddedModules::RequestAxiController(const IResource *axi_port,
					   bool reset_polarity) {
  axi_ports_.push_back(make_pair(axi_port, reset_polarity));
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
  // AXI ports
  set<string> controllers;
  for (auto p : axi_ports_) {
    const IResource *res = p.first;
    bool reset_polarity = p.second;
    string name = axi::AxiPort::ControllerName(*res, p.second);
    if (controllers.find(name) != controllers.end()) {
      continue;
    }
    axi::AxiPort::WriteController(*res, reset_polarity, os);
    controllers.insert(name);
  }
  return true;
}

InternalSRAM *EmbeddedModules::RequestInternalSRAM(const Module &mod,
						   const IResource &res,
						   int num_ports) {
  InternalSRAM *sram = new InternalSRAM(mod, res, num_ports);
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
  is << "  " << name
     << " inst_" << tab_.GetITable()->GetId() << "_" << name << "(";
  is << "." << params->GetEmbeddedModuleClk() << "(" << ports->GetClk() << "), "
     << "." << params->GetEmbeddedModuleReset() << "(" << ports->GetReset() << ")";
  ostream &rs = tab_.InitialValueSectionStream();
  ostream &ws = tmpl_->GetStream(kInsnWireDeclSection);
  string ack_pin = params->GetEmbeddedModuleAck();
  if (!ack_pin.empty()) {
    string ack_wire = AckWireName(*params);
    ws << "  wire " << ack_wire << ";\n";
    is << ", ." << ack_pin << "(" << ack_wire << ")";
  }
  string req_pin = params->GetEmbeddedModuleReq();
  if (!req_pin.empty()) {
    string req_reg = ReqRegName(*params);
    ws << "  reg " << req_reg << ";\n";
    rs << "      " << req_reg << " <= 0;\n";
    is << ", ." << req_pin << "(" << req_reg << ")";
  }
  vector<string> args = params->GetEmbeddedModuleArgs();
  CHECK(args.size() == res_.input_types_.size());
  for (int i = 0; i < args.size(); ++i) {
    string &arg = args[i];
    string n = ArgRegName(*params, i);
    ws << "  reg " << Table::ValueWidthSpec(res_.input_types_[i]) << " " << n << ";\n";
    rs << "      " << n << " <= 0;\n";
    is << ", ." << arg << "(" << n << ")";
  }
  is << ");\n";
}

void EmbeddedResource::BuildInsn(IInsn *insn, State *st) {
  static const char I[] = "          ";
  string insn_st = InsnWriter::MultiCycleStateName(*(insn->GetResource()));
  ostream &os = st->StateBodySectionStream();
  os << I << "// Embedded module\n";
  auto *params = res_.GetParams();
  string req = params->GetEmbeddedModuleReq();
  if (req.empty()) {
    os << I << "// (no req port)\n";
    return;
  }
  os << I << "if (" << insn_st << " == 0) begin\n"
     << I << "  " << insn_st << " <= 1;\n";
  os << I << "  " << ReqRegName(*params) << " <= 1;\n";
  vector<string> args = params->GetEmbeddedModuleArgs();
  CHECK(args.size() == insn->inputs_.size());
  for (int i = 0; i < args.size(); ++i) {
    string n = ArgRegName(*params, i);
    os << I << "  " << n << " <= "
       << InsnWriter::RegisterValue(*insn->inputs_[i], tab_.GetNames()) << ";\n";
  }
  os << I << "end\n";
  string ack = AckWireName(*params);
  os << I << "if (" << insn_st << " == 1) begin\n";
  if (!ack.empty()) {
    os << I << "if (" << ack << " == 1) begin\n";
    os << I << "  " << ReqRegName(*params) << " <= 0;\n";
  }
  os << I << "  " << insn_st << " <= 3;\n";
  if (!ack.empty()) {
    os << I << "end\n";
  } else {
    os << I << "  " << ReqRegName(*params) << " <= 0;\n";
  }
  os << I << "end\n";
}

string EmbeddedResource::ArgRegName(const ResourceParams &params, int nth) {
  string name = params.GetEmbeddedModuleName();
  return "arg_" + Util::Itoa(tab_.GetITable()->GetId()) + "_" +
    Util::Itoa(nth) + "_" + name;
}

string EmbeddedResource::AckWireName(const ResourceParams &params) {
  string ack = params.GetEmbeddedModuleAck();
  if (ack.empty()) {
    return ack;
  }
  string name = params.GetEmbeddedModuleName();
  return "ack_" + Util::Itoa(tab_.GetITable()->GetId()) + "_" + name;
}

string EmbeddedResource::ReqRegName(const ResourceParams &params) {
  string name = params.GetEmbeddedModuleName();
  return "req_" + Util::Itoa(tab_.GetITable()->GetId()) + "_" + name;
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
