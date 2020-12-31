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

InsnCondition::~InsnCondition() {
  STLDeleteSecondElements(&cond_value_info_);
  STLDeleteSecondElements(&cond_reg_info_);
}

bool InsnCondition::Build(OptimizerLog *log) {
  for (IState *st : lb_->GetStates()) {
    states_.insert(st);
  }
  CollectBranches();
  for (IState *st : branches_) {
    PropagateCondValue(st);
  }
  CollectSideEffectInsns();
  BuildConditionRegInfo();
  Dump(log);
  return true;
}

void InsnCondition::Dump(OptimizerLog *log) {
  for (IState *br : branches_) {
    log->State(br) << "X";
  }
  for (auto &p : cond_value_info_) {
    ostream &os = log->State(p.first);
    InsnConditionValueInfo *info = p.second;
    for (auto &q : info->cond_to_value_) {
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
    InsnConditionValueInfo *info = cond_value_info_[st];
    if (info == nullptr) {
      info = new InsnConditionValueInfo();
      cond_value_info_[st] = info;
    }
    info->cond_to_value_[cond_reg] = p.second;
  }
  ConditionRegInfo *reg_info = GetCondRegInfo(cond_reg);
  reg_info->branch_st_ = branch_st;
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
    auto it = cond_value_info_.find(st);
    if (it == cond_value_info_.end()) {
      continue;
    }
    InsnConditionValueInfo *info = it->second;
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
      info->insns_.push_back(insn);
    }
  }
}

ConditionRegInfo *InsnCondition::GetCondRegInfo(IRegister *cond_reg) {
  ConditionRegInfo *reg_info = nullptr;
  auto it = cond_reg_info_.find(cond_reg);
  if (it == cond_reg_info_.end()) {
    reg_info = new ConditionRegInfo();
    reg_info->last_use_ = -1;
    cond_reg_info_[cond_reg] = reg_info;
  } else {
    reg_info = it->second;
  }
  return reg_info;
}

void InsnCondition::BuildConditionRegInfo() {
  for (auto &p : cond_value_info_) {
    InsnConditionValueInfo *cvinfo = p.second;
    int st_index = lb_->GetIndexFromState(p.first);
    for (auto &r : cvinfo->cond_to_value_) {
      IRegister *cond_reg = r.first;
      ConditionRegInfo *reg_info = GetCondRegInfo(cond_reg);
      if (reg_info->last_use_ > st_index) {
        reg_info->last_use_ = st_index;
      }
    }
  }
}

}  // namespace pipeline
}  // namespace opt
}  // namespace iroha
