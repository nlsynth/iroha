#include "writer/verilog/shared_reg_ext_writer.h"

namespace iroha {
namespace writer {
namespace verilog {

SharedRegExtWriter::SharedRegExtWriter(const IResource &res,
				       const Table &table)
  : Resource(res, table) {
}

void SharedRegExtWriter::BuildResource() {
}

void SharedRegExtWriter::BuildInsn(IInsn *insn, State *st) {
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
