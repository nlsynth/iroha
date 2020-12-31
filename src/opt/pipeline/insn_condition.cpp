#include "opt/pipeline/insn_condition.h"

#include "design/design_util.h"
#include "iroha/i_design.h"
#include "iroha/resource_attr.h"
#include "iroha/stl_util.h"
#include "opt/loop/loop_block.h"
#include "opt/optimizer_log.h"

namespace iroha {
namespace opt {
namespace pipeline {

InsnCondition::InsnCondition(loop::LoopBlock *lb)
    : tab_(lb->GetTable()), lb_(lb) {}

InsnCondition::~InsnCondition() { STLDeleteSecondElements(&cond_info_); }

bool InsnCondition::Build(OptimizerLog *log) {
  for (IState *st : lb_->GetStates()) {
    states_.insert(st);
  }
  CollectBranches();
  for (IState *st : branches_) {
    PropagateCondValue(st);
  }
  CollectSideEffectInsns();
  Dump(log);
  return true;
}

void InsnCondition::Dump(OptimizerLog *log) {
  for (IState *br : branches_) {
    log->State(br) << "X";
  }
  for (auto &p : cond_info_) {
    ostream &os = log->State(p.first);
    InsnConditionInfo *info = p.second;
    for (auto &q : info->cond_to_value) {
      os << " " << q.first->GetId() << ":" << q.second;
    }
  }
}

bool InsnCondition::InLoop(IState *st) { return (states_.count(st) == 1); }

bool InsnCondition::IsEntry(IState *st) {
  auto sts = lb_->GetStates();
  return (sts[0] == st);
}

void InsnCondition::CollectBranches() {
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
    }
  }
}

void InsnCondition::PropagateCondValue(IState *branch_st) {
  IInsn *tr = DesignUtil::FindTransitionInsn(branch_st);
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
  IRegister *cond_reg = tr->inputs_[0];
  for (auto &p : state_values) {
    IState *st = p.first;
    InsnConditionInfo *info = cond_info_[st];
    if (info == nullptr) {
      info = new InsnConditionInfo();
      cond_info_[st] = info;
    }
    info->cond_to_value[cond_reg] = p.second;
  }
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

void InsnCondition::CollectSideEffectInsns() {
  for (IState *st : lb_->GetStates()) {
    auto it = cond_info_.find(st);
    if (it == cond_info_.end()) {
      continue;
    }
    InsnConditionInfo *info = it->second;
    vector<IInsn *> side_effect_insns;
    for (IInsn *insn : st->insns_) {
      if (ResourceAttr::IsSideEffectInsn(insn)) {
        side_effect_insns.push_back(insn);
      }
    }
    if (side_effect_insns.size() == 0) {
      continue;
    }
    for (IInsn *insn : side_effect_insns) {
      info->insns_.push_back(make_pair(st, insn));
    }
  }
}

}  // namespace pipeline
}  // namespace opt
}  // namespace iroha
