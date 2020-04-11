#include "opt/unroll/unroller.h"

#include "design/design_util.h"
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
}

}  // namespace unroll
}  // namespace opt
}  // namespace iroha
