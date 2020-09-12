#include "opt/clean/empty_state.h"

#include "design/design_util.h"
#include "iroha/i_design.h"
#include "iroha/logging.h"

namespace iroha {
namespace opt {
namespace clean {

CleanEmptyStatePhase::~CleanEmptyStatePhase() {}

Pass *CleanEmptyStatePhase::Create() { return new CleanEmptyStatePhase(); }

bool CleanEmptyStatePhase::ApplyForTable(const string &key, ITable *table) {
  CleanEmptyState shrink(table, annotation_);
  return shrink.Perform();
}

CleanEmptyState::CleanEmptyState(ITable *table, DebugAnnotation *annotation)
    : table_(table), annotation_(annotation) {
  transition_ = DesignUtil::FindTransitionResource(table);
}

bool CleanEmptyState::Perform() {
  // Finds dead states.
  for (IState *st : table_->states_) {
    if (st == table_->GetInitialState()) {
      continue;
    }
    if (IsEmptyState(st)) {
      IInsn *tr_insn = DesignUtil::FindInsnByResource(st, transition_);
      if (tr_insn == nullptr || tr_insn->target_states_.size() == 0 ||
          (tr_insn->target_states_.size() == 1 &&
           tr_insn->target_states_[0] == st)) {
        // skips if this is a terminal state.
        continue;
      }
      dead_st_.insert(st);
    }
  }
  // Finds an alternative state for a dead state.
  map<IState *, IState *> dead_to_next;
  for (IState *st : dead_st_) {
    dead_to_next[st] = GetNextIfDead(st);
  }
  // Updates transition targets.
  for (IState *st : table_->states_) {
    IInsn *tr_insn = DesignUtil::FindInsnByResource(st, transition_);
    if (tr_insn == nullptr) {
      continue;
    }
    for (auto it = tr_insn->target_states_.begin();
         it != tr_insn->target_states_.end(); ++it) {
      IState *tst = *it;
      auto jt = dead_to_next.find(tst);
      if (jt != dead_to_next.end()) {
        *it = jt->second;
      }
    }
  }
  return true;
}

bool CleanEmptyState::IsEmptyState(IState *st) {
  bool has_insn = false;
  for (IInsn *insn : st->insns_) {
    if (insn->GetResource() == transition_) {
      if (insn->target_states_.size() > 1) {
        // Consider branch as an insn.
        has_insn = true;
      }
    } else {
      has_insn = true;
    }
  }
  return !has_insn;
}

IState *CleanEmptyState::GetNextIfDead(IState *st) {
  if (dead_st_.find(st) == dead_st_.end()) {
    return st;
  }
  IInsn *tr_insn = DesignUtil::FindInsnByResource(st, transition_);
  if (tr_insn == nullptr || tr_insn->target_states_.size() == 0) {
    // This is dead and also terminal.
    return st;
  }
  CHECK(tr_insn->target_states_.size() == 1);
  return GetNextIfDead(tr_insn->target_states_[0]);
}

}  // namespace clean
}  // namespace opt
}  // namespace iroha
