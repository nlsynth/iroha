#include "opt/clean/unreachable_state.h"

#include "iroha/i_design.h"
#include "opt/opt_util.h"

namespace iroha {
namespace opt {
namespace clean {

CleanUnreachableStatePhase::~CleanUnreachableStatePhase() {
}

Phase *CleanUnreachableStatePhase::Create() {
  return new CleanUnreachableStatePhase();
}

bool CleanUnreachableStatePhase::ApplyForTable(ITable *table) {
  set<IState *> reachables;
  OptUtil::CollectReachableStates(table, &reachables);
  vector<IState *> reachable_states;
  for (IState *st :table->states_) {
    if (reachables.find(st) != reachables.end()) {
      reachable_states.push_back(st);
    }
  }
  table->states_ = reachable_states;
  return true;
}
  
}  // namespace clean
}  // namespace opt
}  // namespace iroha

