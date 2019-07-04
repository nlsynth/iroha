#include "writer/dot_writer.h"

#include "iroha/dot.h"

namespace iroha {
namespace writer {

DotWriter::DotWriter(const IDesign *design, ostream &os) : os_(os) {
}

void DotWriter::Write() {
  Dot d;
  d.Output(os_);
}

}  // namespace writer
}  // namespace iroha
