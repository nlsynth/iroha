#include "writer/verilog/verilog_writer.h"

#include "design/util.h"
#include "iroha/logging.h"
#include "iroha/i_design.h"
#include "iroha/stl_util.h"
#include "writer/verilog/embed.h"
#include "writer/verilog/module.h"

namespace iroha {
namespace verilog {

VerilogWriter::VerilogWriter(const IDesign *design, ostream &os)
  : design_(design), os_(os), embed_(new Embed) {
}

VerilogWriter::~VerilogWriter() {
}

void VerilogWriter::Write() {
  os_ << "// Generated from " << PACKAGE << "-" << VERSION << ".\n\n";

  IModule *root = DesignUtil::GetRootModule(design_);
  BuildModules(root);
  BuildHierarchy();
  if (!embed_->Write(os_)) {
    LOG(ERROR) << "Failed to write embedded modules.";
  }
  for (auto *mod : ordered_modules_) {
    mod->Write(os_);
  }
  STLDeleteSecondElements(&modules_);
}

void VerilogWriter::BuildModules(const IModule *imod) {
  vector<IModule *> children = DesignUtil::GetChildModules(imod);
  for (IModule *child : children) {
    BuildModules(child);
  }
  Module *mod = new Module(imod, embed_.get());
  mod->Build();
  modules_[imod] = mod;
  ordered_modules_.push_back(mod);
}

void VerilogWriter::BuildHierarchy() {
  for (auto *mod : ordered_modules_) {
    vector<IModule *> child_imods =
      DesignUtil::GetChildModules(mod->GetIModule());
    vector<Module *> mods;
    for (auto *imod : child_imods) {
      mods.push_back(modules_[imod]);
    }
    mod->BuildChildModuleSection(mods);
  }
}

}  // namespace verilog
}  // namespace iroha
