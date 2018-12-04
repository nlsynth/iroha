#include "opt/wire/explorer.h"

namespace iroha {
namespace opt {
namespace wire {

Explorer::Explorer(WirePlanSet *wps) : wps_(wps) {
}

void Explorer::SetInitialAllocation() {
  // Do nothing.
}

bool Explorer::MaySetNextAllocationPlan() {
  return false;
}

}  // namespace wire
}  // namespace opt
}  // namespace iroha
