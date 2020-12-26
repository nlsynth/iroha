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
      counter_reg_(reg),
      counter_initial_reg_(nullptr),
      initial_assign_st_(nullptr),
      compare_st_(nullptr),
      compare_insn_(nullptr),
      branch_insn_(nullptr),
      exit_st_(nullptr),
      increment_insn_(nullptr),
      loop_count_(0) {}

bool LoopBlock::Build() {
  // Finds
  // * Initial 0-value assignment to the index.
  // * Compare with a constant.
  // * Branch to exit or enter the loop.
  // * Increment the index.
  auto ap = FindInitialAssign();
  initial_assign_st_ = ap.first;
  if (initial_assign_st_ == nullptr) {
    return false;
  }
  counter_initial_reg_ = FindInitialValue(ap.second);
  auto cp = FindCompareInsn(initial_assign_st_);
  compare_st_ = cp.first;
  compare_insn_ = cp.second;
  if (compare_insn_ == nullptr) {
    return false;
  }
  loop_count_ = compare_insn_->inputs_[0]->GetInitialValue().GetValue0();
  IState *tr_st = FindTransition(compare_st_, compare_insn_);
  if (tr_st == nullptr) {
    return false;
  }
  branch_insn_ = DesignUtil::FindTransitionInsn(tr_st);
  exit_st_ = branch_insn_->target_states_[0];
  if (exit_st_ == nullptr) {
    return false;
  }
  CollectLoopStates(exit_st_, compare_st_);
  FindIncrementInsn();
  return true;
}

int LoopBlock::GetLoopCount() { return loop_count_; }

ITable *LoopBlock::GetTable() { return tab_; }

IRegister *LoopBlock::GetInitialCounterValue() {
  if (counter_initial_reg_ != nullptr) {
    return counter_initial_reg_;
  }
  return counter_reg_;
}

IRegister *LoopBlock::GetCounterRegister() { return counter_reg_; }

vector<IState *> &LoopBlock::GetStates() { return states_; }

pair<IState *, IInsn *> LoopBlock::FindCompareInsn(IState *initial_assign_st) {
  IState *compare_st = nullptr;
  IInsn *compare_insn = nullptr;
  for (IState *st = initial_assign_st; st != nullptr;
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
  return make_pair(compare_st, compare_insn);
}

pair<IState *, IInsn *> LoopBlock::FindInitialAssign() {
  IState *res_st = nullptr;
  IInsn *res_insn = nullptr;
  for (IState *st : tab_->states_) {
    for (IInsn *insn : st->insns_) {
      IState *assign_st = CheckInitialAssign(st, insn);
      if (assign_st != nullptr && res_st != nullptr) {
        // Only one is allowed.
        return make_pair(nullptr, nullptr);
      }
      if (assign_st != nullptr) {
        res_st = assign_st;
        res_insn = insn;
      }
    }
  }
  return make_pair(res_st, res_insn);
}

IRegister *LoopBlock::FindInitialValue(IInsn *insn) {
  IResourceClass *rc = insn->GetResource()->GetClass();
  if (!resource::IsSelect(*rc)) {
    return nullptr;
  }
  for (int i = 1; i < insn->inputs_.size(); ++i) {
    IRegister *reg = insn->inputs_[i];
    // WIP: Actual initial value is given from assign from constant.
    if (reg->HasInitialValue()) {
      return reg;
    }
  }
  return nullptr;
}

IState *LoopBlock::CheckInitialAssign(IState *st, IInsn *insn) {
  IResourceClass *rc = insn->GetResource()->GetClass();
  if (resource::IsSelect(*rc)) {
    // after PHI removal.
    // reg <- select(...)
    if (insn->outputs_[0] != counter_reg_) {
      return nullptr;
    }
    return st;
  }
  if (!resource::IsSet(*rc)) {
    return nullptr;
  }
  if (insn->outputs_.size() != 1 || insn->inputs_.size() != 1) {
    return nullptr;
  }
  if (insn->outputs_[0] != counter_reg_) {
    return nullptr;
  }
  IRegister *rhs = insn->inputs_[0];
  if (!rhs->IsConst()) {
    return nullptr;
  }
  return st;
}

void LoopBlock::FindIncrementInsn() {
  for (IState *st : states_) {
    for (IInsn *insn : st->insns_) {
      IResourceClass *rc = insn->GetResource()->GetClass();
      if (resource::IsAdd(*rc)) {
        if (insn->outputs_[0] == counter_reg_) {
          increment_insn_ = insn;
          return;
        }
      }
    }
  }
}

IInsn *LoopBlock::CompareResult(IInsn *insn) {
  IResourceClass *rc = insn->GetResource()->GetClass();
  if (!resource::IsGt(*rc)) {
    return nullptr;
  }
  // CONST >GT> counter
  if (insn->inputs_[0]->IsConst() && insn->inputs_[1] == counter_reg_) {
    return insn;
  }
  return nullptr;
}

IState *LoopBlock::FindTransition(IState *compare_st, IInsn *compare_insn) {
  IRegister *compare_reg = compare_insn->outputs_[0];
  for (IState *st = compare_st; st != nullptr;
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

IInsn *LoopBlock::GetIncrementInsn() { return increment_insn_; }

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
