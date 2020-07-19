#include "writer/verilog/embedded_modules.h"

#include "iroha/i_design.h"
#include "iroha/logging.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"
#include "iroha/stl_util.h"
#include "writer/verilog/axi/master_port.h"
#include "writer/verilog/axi/slave_port.h"
#include "writer/verilog/internal_sram.h"
#include "writer/verilog/module.h"
#include "writer/verilog/wire/mux.h"
#include "writer/verilog/wire/wire_set.h"

#include <set>

namespace iroha {
namespace writer {
namespace verilog {

EmbeddedModules::~EmbeddedModules() {
  STLDeleteValues(&wire_sets_);
}

void EmbeddedModules::RequestModule(const ResourceParams &params) {
  string name = params.GetEmbeddedModuleFileName();
  if (!name.empty()) {
    string rst = params.GetEmbeddedModuleReset();
    files_.insert(make_pair(name, rst));
  }
}

void EmbeddedModules::RequestAxiMasterController(const IResource *axi_port) {
  axi_ports_.push_back(axi_port);
}

void EmbeddedModules::RequestAxiSlaveController(const IResource *axi_port) {
  axi_ports_.push_back(axi_port);
}

void EmbeddedModules::RequestWireMux(const wire::WireSet *wire_set) {
  wire_sets_.push_back(wire_set);
}

bool EmbeddedModules::Write(bool reset_polarity, ostream &os) {
  // Files
  for (auto &it : files_) {
    bool ok = CopyFile(it.first, it.second, reset_polarity, os);
    if (!ok) {
      return false;
    }
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
  // Wire set muxes
  for (const auto *ws : wire_sets_) {
    wire::Mux::Write(ws, os);
  }
  return true;
}

bool EmbeddedModules::CopyFile(const string &fn, const string &rst,
			       bool reset_polarity, ostream &os) {
  istream *ifs = Util::OpenFile(fn);
  if (ifs == nullptr) {
    LOG(ERROR) << "Failed to open: " << fn;
    return false;
  }
  unique_ptr<istream> deleter(ifs);
  os << "// Copied from " << fn;
  if (!reset_polarity) {
    os << " (inverted reset polarity)";
  }
  os << "\n";
  while (!ifs->eof()) {
    string line;
    getline(*ifs, line);
    if (!reset_polarity) {
      // TODO: Allow negative reset to be imported too.
      // rewrite "(rst)" to "(!rst)"
      string r = "(" + rst + ")";
      int pos = line.find(r);
      if (pos != string::npos) {
	line = line.replace(pos, r.size(), "(!" + rst + ")");
      }
    }
    os << line << "\n";
  }
  os << "//\n";
  return true;
}

InternalSRAM *EmbeddedModules::RequestInternalSRAM(const Module &mod,
						   const IArray &arr,
						   int num_ports) {
  InternalSRAM *sram = new InternalSRAM(arr, num_ports, mod.GetResetPolarity());
  srams_.push_back(sram);
  return sram;
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
