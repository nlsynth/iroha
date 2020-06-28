#include "writer/verilog/ext_io_accessor.h"

namespace iroha {
namespace writer {
namespace verilog {

ExtIOAccessor::ExtIOAccessor(const IResource &res, const Table &table)
  : Resource(res, table) {
}

void ExtIOAccessor::BuildResource() {
}

void ExtIOAccessor::BuildInsn(IInsn *insn, State *st) {
}

void ExtIOAccessor::CollectNames(Names *names) {
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

