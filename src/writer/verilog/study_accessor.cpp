#include "writer/verilog/study_accessor.h"

namespace iroha {
namespace writer {
namespace verilog {

StudyAccessor::StudyAccessor(const IResource &res, const Table &table)
  : Resource(res, table) {
}

void StudyAccessor::BuildResource() {
}

void StudyAccessor::BuildInsn(IInsn *insn, State *st) {
}

void StudyAccessor::CollectNames(Names *names) {
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
