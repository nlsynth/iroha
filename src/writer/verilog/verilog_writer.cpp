#include "writer/verilog/verilog_writer.h"

namespace iroha {
namespace verilog {

VerilogWriter::VerilogWriter(const IDesign *design, ostream &os)
  : design_(design), os_(os) {
}

void VerilogWriter::Write() {
  os_ << "// Verilog writer: to be implemented.\n";
}

}  // namespace verilog
}  // namespace iroha
