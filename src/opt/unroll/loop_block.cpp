#include "opt/unroll/loop_block.h"

#include "iroha/resource_class.h"
#include "iroha/i_design.h"

namespace iroha {
namespace opt {
namespace unroll {

LoopBlock::LoopBlock(ITable *tab, IRegister *reg)
  : tab_(tab), reg_(reg), initial_assign_(nullptr) {
}

bool LoopBlock::Build() {
  // Finds
  // * Initial 0-value assignment to the index.
  // WIP.
  // * Compare with a constant.
  // * Branch to exit or enter the loop.
  // * Increment the index.
  for (IState *st : tab_->states_) {
    for (IInsn *insn : st->insns_) {
      FindInitialAssign(insn);
    }
  }
  return false;
}

void LoopBlock::FindInitialAssign(IInsn *insn) {
  IResource *res = insn->GetResource();
  if (!resource::IsSet(*(res->GetClass()))) {
    return;
  }
  if (insn->outputs_.size() != 1 ||
      insn->inputs_.size() != 1) {
    return;
  }
  if (insn->outputs_[0] != reg_) {
    return;
  }
  IRegister *rhs = insn->inputs_[0];
  if (!rhs->IsConst()) {
    return;
  }
  initial_assign_ = insn;
}

}  // namespace unroll
}  // namespace opt
}  // namespace iroha
