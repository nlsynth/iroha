#include "opt/bb_collector.h"

#include "iroha/i_design.h"
#include "opt/bb_set.h"
#include "opt/opt_util.h"

namespace iroha {
namespace opt {

BBCollector::BBCollector(ITable *table) : table_(table) {
}

BBSet *BBCollector::Create() {
  BBSet *bs = new BBSet;
  set<IState *> reachables;
  OptUtil::CollectReachableStates(table_, &reachables);
  // TODO(yt76): Implement the rest.
  return bs;
}
  
}  // namespace opt
}  // namespace iroha
