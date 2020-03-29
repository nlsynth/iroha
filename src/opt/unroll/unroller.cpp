#include "opt/unroll/unroller.h"

namespace iroha {
namespace opt {
namespace unroll {

Unroller::Unroller(ITable *tab, LoopBlock *lb, int count)
  : tab_(tab), lb_(lb), count_(count) {
}

bool Unroller::Unroll() {
  return true;
}

}  // namespace unroll
}  // namespace opt
}  // namespace iroha
