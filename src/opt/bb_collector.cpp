#include "opt/bb_collector.h"

#include "design/design_util.h"
#include "iroha/i_design.h"
#include "opt/bb_set.h"
#include "opt/opt_util.h"

namespace iroha {
namespace opt {

BBCollector::BBCollector(ITable *table, DebugAnnotation *annotation)
  : table_(table), annotation_(annotation), bset_(new BBSet(table)) {
  tr_ = DesignUtil::FindTransitionResource(table_);
}

BBSet *BBCollector::Create() {
  CollectEntries();
  for (IState *es : bb_entries_) {
    CollectBBsFromEntry(es);
  }
  // Additional information.
  SetStateBBMap();
  SetBBTransition();
  IState *initial_st = table_->GetInitialState();
  if (initial_st != nullptr) {
    bset_->initial_bb_ = bset_->state_to_bb_[initial_st];
  }
  if (annotation_ != nullptr) {
    Annotate();
  }
  return bset_;
}

void BBCollector::CollectEntries() {
  set<IState *> reachables;
  OptUtil::CollectReachableStates(table_, &reachables);
  OptUtil::CollectTransitionInfo(table_, &transition_info_);
  for (IState *st : reachables) {
    TransitionInfo &ti = transition_info_[st];
    if (ti.nr_branch_ > 1) {
      IInsn *tr_insn = DesignUtil::FindInsnByResource(st, tr_);
      if (tr_insn != nullptr) {
	for (IState *target_st : tr_insn->target_states_) {
	  bb_entries_.insert(target_st);
	}
      }
    }
    if (ti.nr_join_ > 1) {
      bb_entries_.insert(st);
    }
  }
  bb_entries_.insert(table_->GetInitialState());
}

void BBCollector::CollectBBsFromEntry(IState *entry_st) {
  IInsn *tr_insn = DesignUtil::FindInsnByResource(entry_st, tr_);
  if (tr_insn == nullptr) {
    CollectBB(entry_st, nullptr);
  } else {
    set<IState *> targets_seen;
    for (IState *target_st : tr_insn->target_states_) {
      IState *next_st = target_st;
      TransitionInfo &target_ti = transition_info_[target_st];
      if (target_ti.nr_join_ > 1) {
	// multiple incoming path
	next_st = nullptr;
      }
      if (bb_entries_.find(next_st) != bb_entries_.end()) {
	next_st = nullptr;
      }
      if (targets_seen.find(next_st) != targets_seen.end()) {
	continue;
      }
      targets_seen.insert(next_st);
      CollectBB(entry_st, next_st);
    }
  }
}

void BBCollector::CollectBB(IState *entry_st, IState *next_st) {
  BB *bb = new BB();
  bb->states_.push_back(entry_st);
  IState *st = next_st;
  while (st != nullptr) {
    TransitionInfo &ti = transition_info_[st];
    if (ti.nr_join_ > 1 || st == table_->GetInitialState()) {
      break;
    }
    bb->states_.push_back(st);
    if (ti.nr_branch_ > 1) {
      break;
    }
    IState *next = OptUtil::GetOneNextState(st);
    if (next == nullptr|| next == st) {
      break;
    }
    st = next;
  }
  bset_->bbs_.push_back(bb);
  bb->bb_id_ = bset_->bbs_.size();
}

void BBCollector::SetStateBBMap() {
  for (BB *bb : bset_->bbs_) {
    for (IState *st : bb->states_) {
      bset_->state_to_bb_[st] = bb;
    }
  }
}

void BBCollector::SetBBTransition() {
  for (BB *bb : bset_->bbs_) {
    for (IState *st : bb->states_) {
      IInsn *tr_insn = DesignUtil::FindInsnByResource(st, tr_);
      if (tr_insn == nullptr) {
	continue;
      }
      for (IState *target_st : tr_insn->target_states_) {
	BB *target_bb = bset_->state_to_bb_[target_st];
	if (target_bb == bb &&
	    target_st != target_bb->states_[0]) {
	  // Just a transition in this BB. Ignore it.
	  continue;
	}
	bb->next_bbs_.insert(target_bb);
	target_bb->prev_bbs_.insert(bb);
      }
    }
  }
}

void BBCollector::Annotate() {
  bset_->Annotate(annotation_);
}

}  // namespace opt
}  // namespace iroha
