#include "opt/unroll/state_copier.h"

#include "iroha/i_design.h"
#include "opt/unroll/loop_block.h"

namespace iroha {
namespace opt {
namespace unroll {

StateCopier::StateCopier(ITable *tab, LoopBlock *lb, bool is_head)
  : tab_(tab), lb_(lb), is_head_(is_head), continue_st_(nullptr) {
}

void StateCopier::Copy() {
  auto &states = lb_->GetStates();
  for (IState *os : states) {
    IState *ns = new IState(tab_);
    state_copy_map_[os] = ns;
    new_states_.push_back(ns);
    tab_->states_.push_back(ns);
  }
  continue_st_ = new IState(tab_);
  new_states_.push_back(continue_st_);
  tab_->states_.push_back(continue_st_);
  for (IState *os : states) {
    CopyState(os);
  }
  for (IState *os : states) {
    for (IInsn *oinsn : os->insns_) {
      IInsn *ninsn = insn_copy_map_[oinsn];
      for (IInsn *dinsn : oinsn->depending_insns_) {
	ninsn->depending_insns_.push_back(insn_copy_map_[dinsn]);
      }
    }
  }
}

void StateCopier::CopyState(IState *os) {
  IState *ns = state_copy_map_[os];
  IState *orig_compare_st = lb_->GetCompareState();
  IInsn *orig_compare_insn = lb_->GetCompareInsn();
  IInsn *orig_branch_insn = lb_->GetBranchInsn();
  for (IInsn *oinsn : os->insns_) {
    if (oinsn == orig_compare_insn && !is_head_) {
      continue;
    }
    IInsn *ninsn = new IInsn(oinsn->GetResource());
    insn_copy_map_[oinsn] = ninsn;
    // input, output
    ninsn->inputs_ = oinsn->inputs_;
    ninsn->outputs_ = oinsn->outputs_;
    // target states
    for (IState *otarget_st : oinsn->target_states_) {
      IState *ntarget_st = state_copy_map_[otarget_st];
      if (otarget_st == orig_compare_st) {
	ntarget_st = continue_st_;
      }
      if (ntarget_st == nullptr) {
	ntarget_st = lb_->GetExitState();
      }
      ninsn->target_states_.push_back(ntarget_st);
    }
    if (oinsn == orig_branch_insn && !is_head_) {
      // Removes the conditional branch and just proceed to next.
      ninsn->inputs_.clear();
      IState *next = ninsn->target_states_[1];
      ninsn->target_states_.clear();
      ninsn->target_states_.push_back(next);
    }
    ns->insns_.push_back(ninsn);
  }
}

IState *StateCopier::GetInitialState() {
  return new_states_[0];
}

IState *StateCopier::GetContinueState() {
  return continue_st_;
}

}  // namespace unroll
}  // namespace opt
}  // namespace iroha
