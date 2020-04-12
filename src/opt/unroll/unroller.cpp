#include "opt/unroll/unroller.h"

#include "design/design_util.h"
#include "design/design_tool.h"
#include "iroha/i_design.h"
#include "opt/unroll/loop_block.h"
#include "opt/unroll/state_copier.h"

namespace iroha {
namespace opt {
namespace unroll {

Unroller::Unroller(ITable *tab, LoopBlock *lb, int unroll_count)
  : tab_(tab), lb_(lb), unroll_count_(unroll_count) {
  copiers_.reset(new vector<StateCopier *>());
}

bool Unroller::Unroll() {
  if ((lb_->GetLoopCount() % unroll_count_) != 0) {
    return false;
  }
  for (int i = 0; i < unroll_count_; ++i) {
    UnrollOne();
  }
  Reconnect();
  return true;
}

void Unroller::UnrollOne() {
  StateCopier *copier = new StateCopier(tab_, lb_);
  copier->Copy();
  copiers_->push_back(copier);
}

void Unroller::Reconnect() {
  IState *es = lb_->GetEntryAssignState();
  IInsn *tr = DesignUtil::FindTransitionInsn(es);
  tr->target_states_.clear();
  tr->target_states_.push_back(copiers_->at(0)->GetInitialState());
  for (int i = 0; i < copiers_->size() - 1; ++i) {
    StateCopier *cur = copiers_->at(i);
    StateCopier *next = copiers_->at(i + 1);
    DesignTool::AddNextState(cur->GetContinueState(), next->GetInitialState());
  }
  StateCopier *head = copiers_->at(0);
  StateCopier *last = copiers_->at(copiers_->size() - 1);
  DesignTool::AddNextState(last->GetContinueState(), head->GetInitialState());
}

}  // namespace unroll
}  // namespace opt
}  // namespace iroha
