#include "opt/opt_util.h"

#include "design/design_util.h"
#include "iroha/i_design.h"

namespace iroha {
namespace opt {

TransitionInfo::TransitionInfo() : nr_branch_(0), nr_join_(0) {
}

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

void OptUtil::CollectTransitionInfo(ITable *tab,
				    map<IState *, TransitionInfo> *transition_info) {
  IResource *tr = DesignUtil::FindTransitionResource(tab);
  set<IState *> reachables;
  CollectReachableStates(tab, &reachables);
  for (auto st : reachables) {
    TransitionInfo &info = (*transition_info)[st];
    IInsn *tr_insn = DesignUtil::FindInsnByResource(st, tr);
    if (tr_insn) {
      info.nr_branch_ = tr_insn->target_states_.size();
      for (auto target_st : tr_insn->target_states_) {
	TransitionInfo &target_info = (*transition_info)[target_st];
	++target_info.nr_join_;
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

IState *OptUtil::GetOneNextState(IState *cur) {
  ITable *tab = cur->GetTable();
  IResource *tr = DesignUtil::FindTransitionResource(tab);
  IInsn *tr_insn = DesignUtil::FindInsnByResource(cur, tr);
  if (tr_insn == nullptr) {
    return nullptr;
  }
  if (tr_insn->target_states_.size() > 1) {
    return nullptr;
  }
  return tr_insn->target_states_[0];
}

}  // namespace opt
}  // namespace iroha
