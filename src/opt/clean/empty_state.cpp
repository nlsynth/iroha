#include "opt/clean/empty_state.h"

#include "design/design_util.h"
#include "iroha/i_design.h"
#include "iroha/logging.h"
#include "opt/bb_set.h"
#include "opt/debug_annotation.h"

namespace iroha {
namespace opt {
namespace clean {

CleanEmptyStatePhase::~CleanEmptyStatePhase() {
}

Phase *CleanEmptyStatePhase::Create() {
  return new CleanEmptyStatePhase();
}

bool CleanEmptyStatePhase::ApplyForTable(const string &key, ITable *table) {
  CleanEmptyState shrink(table, annotation_);
  return shrink.Perform();
}

CleanEmptyState::CleanEmptyState(ITable *table,  DebugAnnotation *annotation)
  : table_(table), annotation_(annotation) {
  transition_ = DesignUtil::FindTransitionResource(table);
}

bool CleanEmptyState::Perform() {
  unique_ptr<BBSet> bbs(BBSet::Create(table_, annotation_));
  for (BB *bb : bbs->bbs_) {
    ShrinkBB(bb);
  }
  return true;
}

void CleanEmptyState::ShrinkBB(BB *bb) {
  // Doesn't consider states in other BBs.
  dead_st_.clear();
  // Skips first status.
  for (int i = 1; i < bb->states_.size(); ++i) {
    IState *st = bb->states_[i];
    if (IsEmptyState(st)) {
      dead_st_.insert(st);
    }
  }
  for (IState *st : bb->states_) {
    IInsn *tr_insn = DesignUtil::FindInsnByResource(st, transition_);
    if (tr_insn == nullptr) {
      continue;
    }
    for (auto it = tr_insn->target_states_.begin();
	 it != tr_insn->target_states_.end(); ++it) {
      *it = GetNextIfDead(*it);
    }
  }
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
