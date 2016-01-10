#include "opt/bb_collector.h"

#include "design/util.h"
#include "iroha/i_design.h"
#include "opt/bb_set.h"

namespace iroha {
namespace opt {

BBCollector::BBCollector(ITable *table) : table_(table), bbs_(new BBSet) {
  tr_ = DesignUtil::FindTransitionResource(table_);
}

BBSet *BBCollector::Create() {
  CollectEntries();
  for (IState *es : bb_entries_) {
    CollectBBEntry(es);
  }
  return bbs_;
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

void BBCollector::CollectBBEntry(IState *es) {
  IInsn *tr_insn = DesignUtil::FindInsnByResource(es, tr_);
  if (tr_insn == nullptr) {
    CollectBB(es, nullptr);
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
      CollectBB(es, nullptr);
    }
  }
}

void BBCollector::CollectBB(IState *es, IState *next_st) {
  BB *bb = new BB;
  bb->states_.push_back(es);
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
    IState *next = GetOneNextState(st);
    if (next == nullptr|| next == st) {
      break;
    }
    st = next;
  }
  bbs_->bbs_.push_back(bb);
}

IState *BBCollector::GetOneNextState(IState *cur) {
  IInsn *tr_insn = DesignUtil::FindInsnByResource(cur, tr_);
  if (tr_insn == nullptr) {
    return nullptr;
  }
  if (tr_insn->target_states_.size() > 1) {
    return nullptr;
  }
  return tr_insn->target_states_[0];
}

}  // namespace opt
}  // namespace iroha
