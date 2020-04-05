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
    copy_map_[os] = ns;
    new_states_.push_back(ns);
    tab_->states_.push_back(ns);
  }
}

}  // namespace unroll
}  // namespace opt
}  // namespace iroha
