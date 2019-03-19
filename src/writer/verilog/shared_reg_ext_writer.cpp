#include "writer/verilog/shared_reg_ext_writer.h"

#include "iroha/i_design.h"
#include "iroha/resource_params.h"
#include "writer/verilog/shared_reg.h"
#include "writer/verilog/wire/names.h"

namespace iroha {
namespace writer {
namespace verilog {

SharedRegExtWriter::SharedRegExtWriter(const IResource &res,
				       const Table &table)
  : Resource(res, table) {
}

void SharedRegExtWriter::BuildResource() {
  int width =  res_.GetParentResource()->GetParams()->GetWidth();
  auto *klass = res_.GetClass();
  string wrn = SharedReg::GetNameRW(*(res_.GetParentResource()),
				    true);
  string w = wire::Names::AccessorWire(wrn, &res_, "w");
  string wen = wire::Names::AccessorWire(wrn, &res_, "wen");
  AddPortToTop(w, false, true, width);
  AddPortToTop(wen, false, true, 0);
}

void SharedRegExtWriter::BuildInsn(IInsn *insn, State *st) {
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
