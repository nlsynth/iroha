#include "writer/verilog/array_rdata.h"

#include "iroha/i_design.h"
#include "writer/verilog/array.h"
#include "writer/verilog/insn_writer.h"
#include "writer/verilog/table.h"

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
  // Similar to ArrayResource::BuildInsn().
  ostream &ws = tab_.InsnWireValueSectionStream();
  IResource *res = res_.GetParentResource();
  ws << "  assign " << InsnWriter::InsnOutputWireName(*insn, 0)
     << " = " << ArrayResource::SigName(*res, "rdata") << ";\n";
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
