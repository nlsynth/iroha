#include "writer/verilog/embed.h"

#include "iroha/i_design.h"
#include "iroha/resource_params.h"
#include "writer/verilog/ports.h"

#include <fstream>

namespace iroha {
namespace verilog {

Embed::~Embed() {
}

void Embed::RequestModule(const ResourceParams &params) {
  string name = params.GetEmbeddedModuleFileName();
  if (!name.empty()) {
    files_.insert(name);
  }
}

void Embed::BuildModuleInstantiation(const IResource &res,
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

bool Embed::Write(ostream &os) {
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

}  // namespace verilog
}  // namespace iroha
