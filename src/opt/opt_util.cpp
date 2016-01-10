#include "opt/opt_util.h"

#include "design/util.h"
#include "iroha/i_design.h"

namespace iroha {
namespace opt {

void OptUtil::CollectReachableStates(ITable *tab, set<IState *> *reachable) {
  map<IState *, set<IState *>> targets;
  CollectTransitionTargets(tab, &targets);
  set<IState *> frontier;
  frontier.insert(tab->GetInitialState());
  reachable->clear();

  while (frontier.size()) {
    IState *s = *(frontier.begin());
    frontier.erase(s);
    reachable->insert(s);
    auto nexts = targets[s];
    for (auto n : nexts) {
      if (reachable->find(n) == reachable->end()) {
	frontier.insert(n);
      }
    }
  }
}

void OptUtil::CollectTransitionTargets(ITable *tab,
				       map<IState *, set<IState *>> *targets) {
  IResource *tr = DesignUtil::FindTransitionResource(tab);
  for (auto st : tab->states_) {
    IInsn *tr_insn = DesignUtil::FindInsnByResource(st, tr);
    if (tr_insn == nullptr) {
      continue;
    }
    for (auto t : tr_insn->target_states_) {
      (*targets)[st].insert(t);
    }
  }
}

}  // namespace opt
}  // namespace iroha
