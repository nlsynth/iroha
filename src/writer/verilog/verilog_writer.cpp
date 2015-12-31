#include "writer/verilog/verilog_writer.h"

#include "iroha/logging.h"
#include "iroha/i_design.h"
#include "iroha/stl_util.h"
#include "writer/verilog/embed.h"
#include "writer/verilog/module.h"

namespace iroha {
namespace verilog {

VerilogWriter::VerilogWriter(const IDesign *design, ostream &os)
  : design_(design), os_(os) {
}

void VerilogWriter::Write() {
  os_ << "// Generated from " << PACKAGE << "-" << VERSION << ".\n\n";
  vector<Module *> modules;
  Embed embed;

  for (IModule *imod : design_->modules_) {
    Module *mod = new Module(imod, &embed);
    mod->Build();
    modules.push_back(mod);
  }
  if (!embed.Write(os_)) {
    LOG(ERROR) << "Failed to write embedded modules.";
  }
  for (Module *mod : modules) {
    mod->Write(os_);
  }
  STLDeleteValues(&modules);
}

}  // namespace verilog
}  // namespace iroha
