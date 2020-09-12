#include "opt/constant/constant_propagation.h"

#include "design/design_tool.h"
#include "iroha/i_design.h"
#include "iroha/resource_class.h"

namespace iroha {
namespace opt {
namespace constant {

ConstantPropagation::~ConstantPropagation() {}

Pass *ConstantPropagation::Create() { return new ConstantPropagation(); }

bool ConstantPropagation::ApplyForTable(const string &key, ITable *table) {
  IResource *assign = DesignTool::GetOneResource(table, resource::kSet);
  // Before:
  //  Rx <(assign)- Rs
  //  Ry <- Rx
  // After
  //  Rx <(assign)- Rs
  //  Ry <- Rs
  map<IRegister *, IRegister *> dst_to_src;
  for (IState *st : table->states_) {
    for (IInsn *insn : st->insns_) {
      if (insn->GetResource() == assign) {
        dst_to_src[insn->outputs_[0]] = insn->inputs_[0];
      }
    }
  }
  // TODO: Transitive assigns Ry <- Rx, Rz <- Ry
  for (IState *st : table->states_) {
    for (IInsn *insn : st->insns_) {
      for (int i = 0; i < insn->inputs_.size(); ++i) {
        IRegister *src = insn->inputs_[i];
        auto it = dst_to_src.find(src);
        if (it != dst_to_src.end()) {
          insn->inputs_[i] = it->second;
        }
      }
    }
  }
  return true;
}

}  // namespace constant
}  // namespace opt
}  // namespace iroha
