#include "opt/bb_shrink.h"

#include "design/design_util.h"
#include "iroha/i_design.h"
#include "iroha/logging.h"
#include "opt/bb_set.h"
#include "opt/debug_annotation.h"

namespace iroha {
namespace opt {

BBShrinkPhase::~BBShrinkPhase() {
}

Phase *BBShrinkPhase::Create() {
  return new BBShrinkPhase();
}

bool BBShrinkPhase::ApplyForTable(ITable *table) {
  BBShrink shrink(table, annotation_);
  return shrink.Perform();
}

BBShrink::BBShrink(ITable *table,  DebugAnnotation *annotation)
  : table_(table), annotation_(annotation) {
  transition_ = DesignUtil::FindTransitionResource(table);
}

bool BBShrink::Perform() {
  unique_ptr<BBSet> bbs(BBSet::Create(table_, annotation_));
  for (BB *bb : bbs->bbs_) {
    ShrinkBB(bb);
  }
  return true;
}

void BBShrink::ShrinkBB(BB *bb) {
  // Doesn't consider states in other BBs.
  dead_st_.clear();
  // Skips first status.
  for (int i = 1; i < bb->states_.size(); ++i) {
    IState *st = bb->states_[i];
    bool has_insn = false;
    for (IInsn *insn : st->insns_) {
      if (insn->GetResource() == transition_) {
	CHECK(insn->target_states_.size() <= 1);
      } else {
	has_insn = true;
      }
    }
    if (!has_insn) {
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

IState *BBShrink::GetNextIfDead(IState *st) {
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

}  // namespace opt
}  // namespace iroha
