#include "opt/pipeline/insn_condition.h"

#include "design/design_util.h"
#include "iroha/i_design.h"
#include "opt/loop/loop_block.h"
#include "opt/optimizer_log.h"

namespace iroha {
namespace opt {
namespace pipeline {

InsnCondition::InsnCondition(loop::LoopBlock *lb)
    : tab_(lb->GetTable()), lb_(lb) {}

bool InsnCondition::Build(OptimizerLog *log) {
  for (IState *st : lb_->GetStates()) {
    states_.insert(st);
  }
  CollectBranches(log);
  for (IState *st : branches_) {
    PropagateCondValue(st);
  }
  return true;
}

bool InsnCondition::InLoop(IState *st) { return (states_.count(st) == 1); }

bool InsnCondition::IsEntry(IState *st) {
  auto sts = lb_->GetStates();
  return (sts[0] == st);
}

void InsnCondition::CollectBranches(OptimizerLog *log) {
  for (IState *st : lb_->GetStates()) {
    IInsn *tr = DesignUtil::FindTransitionInsn(st);
    int valid_branch = 0;
    for (IState *tst : tr->target_states_) {
      if (InLoop(tst)) {
        ++valid_branch;
      }
    }
    if (valid_branch > 1) {
      branches_.push_back(st);
      log->State(st) << "X";
    }
  }
}

void InsnCondition::PropagateCondValue(IState *st) {
  IInsn *tr = DesignUtil::FindTransitionInsn(st);
  map<IState *, set<int>> values;
  for (int i = 0; i < tr->target_states_.size(); ++i) {
    IState *next_st = tr->target_states_[i];
    set<IState *> reachable;
    CollectReachable(next_st, &reachable);
    for (IState *st : reachable) {
      values[st].insert(i);
    }
  }
  map<IState *, int> state_values;
  for (auto &p : values) {
    if (p.second.size() == 1) {
      int v = *(p.second.begin());
      state_values[p.first] = v;
    }
  }
  // WIP.
}

void InsnCondition::CollectReachable(IState *init_st, set<IState *> *sts) {
  set<IState *> frontier;
  frontier.insert(init_st);
  while (frontier.size() > 0) {
    IState *st = *(frontier.begin());
    frontier.erase(st);
    sts->insert(st);
    IInsn *tr = DesignUtil::FindTransitionInsn(st);
    for (IState *tst : tr->target_states_) {
      if (!InLoop(tst) || IsEntry(tst)) {
        continue;
      }
      frontier.insert(tst);
    }
  }
}

}  // namespace pipeline
}  // namespace opt
}  // namespace iroha
