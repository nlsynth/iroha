#include "opt/unroll/unroller.h"

#include "opt/unroll/loop_block.h"

namespace iroha {
namespace opt {
namespace unroll {

Unroller::Unroller(ITable *tab, LoopBlock *lb, int unroll_count)
  : tab_(tab), lb_(lb), unroll_count_(unroll_count) {
}

bool Unroller::Unroll() {
  if ((lb_->GetLoopCount() % unroll_count_) != 0) {
    return false;
  }
  return true;
}

}  // namespace unroll
}  // namespace opt
}  // namespace iroha
