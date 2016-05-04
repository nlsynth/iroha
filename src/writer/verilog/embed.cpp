#include "writer/verilog/embed.h"

#include "iroha/i_design.h"
#include "iroha/resource_params.h"
#include "writer/module_template.h"
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

void EmbeddedModules::BuildModuleInstantiation(const IResource &res,
					       const Ports &ports,
					       ostream &os) {
  auto *params = res.GetParams();
  string name = params->GetEmbeddedModuleName();
  os << "  // " << name << "\n";
  os << "  " << name << " inst_" << name << "(";
  os << "." << params->GetEmbeddedModuleClk() << "(" << ports.GetClk() << "), "
     << "." << params->GetEmbeddedModuleReset() << "(" << ports.GetReset()
     << "));\n";
}

bool EmbeddedModules::Write(ostream &os) {
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
  return true;
}

EmbeddedResource::EmbeddedResource(const IResource &res, const Table &table)
  : Resource(res, table) {
}

void EmbeddedResource::BuildResource() {
  auto *params = res_.GetParams();
  auto *ports = tab_.GetPorts();
  auto *embed = tab_.GetEmbeddedModules();
  embed->RequestModule(*params);
  ostream &is = tmpl_->GetStream(kEmbeddedInstanceSection);
  embed->BuildModuleInstantiation(res_, *ports, is);
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
