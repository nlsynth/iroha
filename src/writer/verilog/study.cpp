#include "writer/verilog/study.h"

namespace iroha {
namespace writer {
namespace verilog {

Study::Study(const IResource &res, const Table &table) : Resource(res, table) {
}

void Study::BuildResource() {
}

void Study::BuildInsn(IInsn *insn, State *st) {
}

void Study::CollectNames(Names *names) {
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
