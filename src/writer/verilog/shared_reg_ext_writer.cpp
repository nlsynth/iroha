#include "writer/verilog/shared_reg_ext_writer.h"

#include "iroha/i_design.h"
#include "iroha/resource_params.h"
#include "writer/verilog/shared_reg.h"
#include "writer/verilog/table.h"
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
  string w_wire = wire::Names::AccessorWire(wrn, &res_, "w");
  string wen_wire = wire::Names::AccessorWire(wrn, &res_, "wen");
  string w;
  string wen;
  string name = res_.GetParams()->GetPortNamePrefix();
  if (name.empty()) {
    w = wire::Names::AccessorSignalBase(wrn, &res_, "w");
    wen = wire::Names::AccessorSignalBase(wrn, &res_, "wen");
  } else {
    w = name + "_w";
    wen = name + "_wen";
  }
  AddPortToTop(w, false, true, width);
  AddPortToTop(wen, false, true, 0);
  ostream &rvs = tab_.ResourceValueSectionStream();
  rvs << "  // shared-reg-ext-writer\n"
      << "  assign " << w_wire << " = " << w << ";\n"
      << "  assign " << wen_wire << " = " << wen << ";\n";
}

void SharedRegExtWriter::BuildInsn(IInsn *insn, State *st) {
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
