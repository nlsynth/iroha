#include "opt/unroll/state_copier.h"

#include "iroha/i_design.h"
#include "opt/unroll/loop_block.h"

namespace iroha {
namespace opt {
namespace unroll {

StateCopier::StateCopier(ITable *tab, LoopBlock *lb) : tab_(tab), lb_(lb) {
}

void StateCopier::Copy() {
  auto &states = lb_->GetStates();
  for (IState *os : states) {
    IState *ns = new IState(tab_);
    state_copy_map_[os] = ns;
    new_states_.push_back(ns);
    tab_->states_.push_back(ns);
  }
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
  for (IInsn *oinsn : os->insns_) {
    IInsn *ninsn = new IInsn(oinsn->GetResource());
    insn_copy_map_[oinsn] = ninsn;
    // input, output
    ninsn->inputs_ = oinsn->inputs_;
    ninsn->outputs_ = oinsn->outputs_;
    // target states
    for (IState *otarget_st : oinsn->target_states_) {
      IState *ntarget_st = state_copy_map_[otarget_st];
      if (ntarget_st == nullptr) {
	ntarget_st = lb_->GetExitState();
      }
      ninsn->target_states_.push_back(ntarget_st);
    }
    ns->insns_.push_back(ninsn);
  }
}

IState *StateCopier::GetInitialState() {
  return new_states_[0];
}

}  // namespace unroll
}  // namespace opt
}  // namespace iroha