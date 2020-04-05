#include "opt/unroll/loop_block.h"

#include "design/design_util.h"
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
  // * Compare with a constant.
  // * Branch to exit or enter the loop.
  // WIP.
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
  IState *tr_st = FindTransition(compare_st, compare_insn);
  IInsn *tr_insn = DesignUtil::FindTransitionInsn(tr_st);
  IState *exit_state = tr_insn->target_states_[0];
  if (exit_state == nullptr) {
    return false;
  }
  CollectLoopStates(exit_state, compare_st);
  return true;
}

int LoopBlock::GetLoopCount() {
  return loop_count_;
}

vector<IState *> &LoopBlock::GetStates() {
  return states_;
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

IState *LoopBlock::FindTransition(IState *compare_st, IInsn *compare_insn) {
  IRegister *compare_reg = compare_insn->outputs_[0];
  for (IState *st = OptUtil::GetOneNextState(compare_st); st != nullptr;
       st = OptUtil::GetOneNextState(st)) {
    for (IInsn *insn : st->insns_) {
      IResourceClass *rc = insn->GetResource()->GetClass();
      if (!resource::IsTransition(*rc)) {
	continue;
      }
      if (insn->inputs_.size() != 1 || (insn->inputs_[0] != compare_reg)) {
	continue;
      }
      if (insn->target_states_.size() == 2) {
	return st;
      }
    }
  }
  return nullptr;
}

void LoopBlock::CollectLoopStates(IState *exit_st, IState *compare_st) {
  set<IState *> sts;
  OptUtil::CollectReachableStatesWithExclude(tab_, compare_st,
					     exit_st, &sts);
  for (IState *st : tab_->states_) {
    if (sts.find(st) != sts.end()) {
      states_.push_back(st);
    }
  }
}

}  // namespace unroll
}  // namespace opt
}  // namespace iroha
