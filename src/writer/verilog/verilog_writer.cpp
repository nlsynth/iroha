#include "writer/verilog/verilog_writer.h"

#include "iroha/i_design.h"
#include "writer/verilog/module.h"

namespace iroha {
namespace verilog {

VerilogWriter::VerilogWriter(const IDesign *design, ostream &os)
  : design_(design), os_(os) {
}

void VerilogWriter::Write() {
  os_ << "// Generated from " << PACKAGE << "-" << VERSION << ".\n";
  for (IModule *imod : design_->modules_) {
    Module mod(imod);
    mod.Write(os_);
  }
}

}  // namespace verilog
}  // namespace iroha
