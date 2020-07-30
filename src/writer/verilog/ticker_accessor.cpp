#include "writer/verilog/ticker_accessor.h"

namespace iroha {
namespace writer {
namespace verilog {

TickerAccessor::TickerAccessor(const IResource &res, const Table &table)
  : Resource(res, table) {
}

void TickerAccessor::BuildResource() {
}

void TickerAccessor::BuildInsn(IInsn *insn, State *st) {
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
