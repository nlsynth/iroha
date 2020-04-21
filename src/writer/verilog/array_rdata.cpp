#include "writer/verilog/array_rdata.h"

namespace iroha {
namespace writer {
namespace verilog {

ArrayRDataResource::ArrayRDataResource(const IResource &res,
				       const Table &table)
  : Resource(res, table) {
}

void ArrayRDataResource::BuildResource() {
}

void ArrayRDataResource::BuildInsn(IInsn *insn, State *st) {
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
