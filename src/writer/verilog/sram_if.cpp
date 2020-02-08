#include "writer/verilog/sram_if.h"

namespace iroha {
namespace writer {
namespace verilog {

SramIf::SramIf(const IResource &res, const Table &table)
  : Resource(res, table) {
}

void SramIf::BuildResource() {
}

void SramIf::BuildInsn(IInsn *insn, State *st) {
}

void SramIf::CollectNames(Names *names) {
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
