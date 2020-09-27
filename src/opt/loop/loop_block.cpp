#include "opt/loop/loop_block.h"

#include "design/design_util.h"
#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "opt/opt_util.h"
#include "opt/optimizer_log.h"

namespace iroha {
namespace opt {
namespace loop {

LoopBlock::LoopBlock(ITable *tab, IRegister *reg)
    : tab_(tab),
      reg_(reg),
      initial_assign_st_(nullptr),
      compare_st_(nullptr),
      compare_insn_(nullptr),
      branch_insn_(nullptr),
      exit_st_(nullptr),
      loop_count_(0) {}

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
  for (IState *st = initial_assign_st_; st != nullptr;
       st = OptUtil::GetOneNextState(st)) {
    for (IInsn *insn : st->insns_) {
      compare_insn_ = CompareResult(insn);
      if (compare_insn_ != nullptr) {
        compare_st_ = st;
        break;
      }
    }
    if (compare_insn_ != nullptr) {
      break;
    }
  }
  if (compare_insn_ == nullptr) {
    return false;
  }
  loop_count_ = compare_insn_->inputs_[0]->GetInitialValue().GetValue0();
  IState *tr_st = FindTransition(compare_st_, compare_insn_);
  branch_insn_ = DesignUtil::FindTransitionInsn(tr_st);
  exit_st_ = branch_insn_->target_states_[0];
  if (exit_st_ == nullptr) {
    return false;
  }
  CollectLoopStates(exit_st_, compare_st_);
  return true;
}

int LoopBlock::GetLoopCount() { return loop_count_; }

ITable *LoopBlock::GetTable() { return tab_; }

IRegister *LoopBlock::GetRegister() { return reg_; }

vector<IState *> &LoopBlock::GetStates() { return states_; }

void LoopBlock::FindInitialAssign(IState *st, IInsn *insn) {
  IResourceClass *rc = insn->GetResource()->GetClass();
  if (!resource::IsSet(*rc)) {
    return;
  }
  if (insn->outputs_.size() != 1 || insn->inputs_.size() != 1) {
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
  if (insn->inputs_[0]->IsConst() && insn->inputs_[1] == reg_) {
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
  OptUtil::CollectReachableStatesWithExclude(tab_, compare_st, exit_st, &sts);
  for (IState *st : tab_->states_) {
    if (sts.find(st) != sts.end()) {
      states_.push_back(st);
    }
  }
}

IState *LoopBlock::GetEntryAssignState() { return initial_assign_st_; }

IState *LoopBlock::GetExitState() { return exit_st_; }

IState *LoopBlock::GetCompareState() { return compare_st_; }

IInsn *LoopBlock::GetCompareInsn() { return compare_insn_; }

IInsn *LoopBlock::GetBranchInsn() { return branch_insn_; }

void LoopBlock::Annotate(OptimizerLog *log) {
  int n = 0;
  for (IState *st : states_) {
    ostream &os = log->State(st);
    os << "*" << n;
    if (st == compare_st_) {
      os << "-";
    }
    ++n;
  }
}

}  // namespace loop
}  // namespace opt
}  // namespace iroha
