#include "opt/unroll/loop_block.h"

#include "iroha/i_design.h"

namespace iroha {
namespace opt {
namespace unroll {

LoopBlock::LoopBlock(ITable *tab, IRegister *reg) : tab_(tab), reg_(reg) {
}

bool LoopBlock::Build() {
  // WIP. Finds
  // * Initial 0-value assignment to the index.
  // * Compare with a constant.
  // * Branch to exit or enter the loop.
  // * Increment the index.
  for (IState *st : tab_->states_) {
    (void) st;
  }
  return false;
}

}  // namespace unroll
}  // namespace opt
}  // namespace iroha
