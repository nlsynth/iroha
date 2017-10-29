#include "writer/verilog/fifo.h"

namespace iroha {
namespace writer {
namespace verilog {

Fifo::Fifo(const IResource &res, const Table &table) : Resource(res, table) {
}

void Fifo::BuildResource() {
}

void Fifo::BuildInsn(IInsn *insn, State *st) {
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
