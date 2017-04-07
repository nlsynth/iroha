#include "writer/verilog/dataflow_in.h"

#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "writer/module_template.h"
#include "writer/verilog/insn_writer.h"
#include "writer/verilog/module.h"
#include "writer/verilog/shared_reg.h"

namespace iroha {
namespace writer {
namespace verilog {

DataFlowIn::DataFlowIn(const IResource &res, const Table &table)
  : Resource(res, table) {
}

void DataFlowIn::BuildResource() {
}

void DataFlowIn::BuildInsn(IInsn *insn, State *st) {
  IResource *res = insn->GetResource();
  IResource *parent = res->GetParentResource();
  if (insn->outputs_.size() == 1 && parent != nullptr &&
      resource::IsSharedReg(*(parent->GetClass()))) {
    ostream &ws = tmpl_->GetStream(kInsnWireValueSection);
    ws << "  assign " << InsnWriter::InsnOutputWireName(*insn, 0)
       << " = " << SharedReg::RegName(*parent) << ";\n";
  }
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
