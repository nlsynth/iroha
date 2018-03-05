#include "writer/verilog/ext_combinational.h"

namespace iroha {
namespace writer {
namespace verilog {

ExtCombinational::ExtCombinational(const IResource &res, const Table &table)
  : Resource(res, table) {
}

void ExtCombinational::BuildResource() {
}

void ExtCombinational::BuildInsn(IInsn *insn, State *st) {
}

void ExtCombinational::CollectNames(Names *names) {
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
