#include "writer/verilog/embed.h"

#include "iroha/i_design.h"
#include "iroha/logging.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"
#include "writer/verilog/axi/master_port.h"
#include "writer/verilog/axi/slave_port.h"
#include "writer/verilog/internal_sram.h"

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

void EmbeddedModules::RequestAxiMasterController(const IResource *axi_port) {
  axi_ports_.push_back(axi_port);
}

void EmbeddedModules::RequestAxiSlaveController(const IResource *axi_port) {
  axi_ports_.push_back(axi_port);
}

bool EmbeddedModules::Write(bool reset_polarity, ostream &os) {
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
  set<string> sram_names;
  for (InternalSRAM *sram : srams_) {
    string name = sram->GetModuleName();
    if (sram_names.find(name) != sram_names.end()) {
      continue;
    }
    sram->Write(os);
    sram_names.insert(name);
  }
  // AXI ports
  set<string> controllers;
  for (const IResource *res : axi_ports_) {
    string name;
    if (resource::IsAxiMasterPort(*(res->GetClass()))) {
      name = axi::MasterPort::ControllerName(*res);
      if (controllers.find(name) != controllers.end()) {
	continue;
      }
      axi::MasterPort::WriteController(*res, reset_polarity, os);
    } else {
      name = axi::SlavePort::ControllerName(*res);
      if (controllers.find(name) != controllers.end()) {
	continue;
      }
      axi::SlavePort::WriteController(*res, reset_polarity, os);
    }
    controllers.insert(name);
  }
  return true;
}

InternalSRAM *EmbeddedModules::RequestInternalSRAM(const Module &mod,
						   const IArray &arr,
						   int num_ports) {
  InternalSRAM *sram = new InternalSRAM(mod, arr, num_ports);
  srams_.push_back(sram);
  return sram;
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
