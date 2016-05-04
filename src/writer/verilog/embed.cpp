#include "writer/verilog/embed.h"

#include "iroha/i_design.h"
#include "iroha/resource_params.h"
#include "writer/module_template.h"
#include "writer/verilog/internal_sram.h"
#include "writer/verilog/module.h"
#include "writer/verilog/ports.h"
#include "writer/verilog/table.h"

#include <fstream>

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
    os << "// Copied from " << s << "\n";
    ifstream *ifs = new ifstream(s);
    if (ifs->fail()) {
      delete ifs;
      return false;
    }
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
     << "." << params->GetEmbeddedModuleReset() << "(" << ports->GetReset()
     << "));\n";
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
