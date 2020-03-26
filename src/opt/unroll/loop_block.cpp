#include "opt/unroll/loop_block.h"

#include "iroha/resource_class.h"
#include "iroha/i_design.h"
#include "opt/opt_util.h"

namespace iroha {
namespace opt {
namespace unroll {

LoopBlock::LoopBlock(ITable *tab, IRegister *reg)
  : tab_(tab), reg_(reg), initial_assign_st_(nullptr), loop_count_(0) {
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
      FindInitialAssign(st, insn);
    }
  }
  if (initial_assign_st_ == nullptr) {
    return false;
  }
  IInsn *compare_insn = nullptr;
  IState *compare_st = nullptr;
  for (IState *st = initial_assign_st_; st != nullptr;
       st = OptUtil::GetOneNextState(st)) {
    for (IInsn *insn : st->insns_) {
      compare_insn = CompareResult(insn);
      if (compare_insn != nullptr) {
	compare_st = st;
	break;
      }
    }
    if (compare_insn != nullptr) {
      break;
    }
  }
  if (compare_insn == nullptr) {
    return false;
  }
  loop_count_ = compare_insn->inputs_[0]->GetInitialValue().GetValue0();
  for (IState *st = OptUtil::GetOneNextState(compare_st); st != nullptr;
       st = OptUtil::GetOneNextState(st)) {
    for (IInsn *insn : st->insns_) {
      (void) insn;
    }
  }
  return false;
}

void LoopBlock::FindInitialAssign(IState *st, IInsn *insn) {
  IResourceClass *rc = insn->GetResource()->GetClass();
  if (!resource::IsSet(*rc)) {
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
  initial_assign_st_ = st;
}

IInsn *LoopBlock::CompareResult(IInsn *insn) {
  IResourceClass *rc = insn->GetResource()->GetClass();
  if (!resource::IsGt(*rc)) {
    return nullptr;
  }
  // CONST >GT> counter
  if (insn->inputs_[0]->IsConst() &&
      insn->inputs_[1] == reg_) {
    return insn;
  }
  return nullptr;
}

}  // namespace unroll
}  // namespace opt
}  // namespace iroha
