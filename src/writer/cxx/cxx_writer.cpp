#include "writer/cxx/cxx_writer.h"

namespace iroha {
namespace writer {
namespace cxx {

CxxWriter::CxxWriter(const IDesign *design, ostream &os)
  : design_(design), os_(os) {
}

void CxxWriter::Write() {
  os_ << "// Generated from " << PACKAGE << "-" << VERSION << ".\n\n";
}

}  // namespace cxx
}  // namespace writer
}  // namespace iroha
