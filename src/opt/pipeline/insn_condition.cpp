#include "opt/pipeline/insn_condition.h"

#include <utility>

#include "design/design_util.h"
#include "iroha/i_design.h"
#include "iroha/logging.h"
#include "iroha/resource_attr.h"
#include "iroha/stl_util.h"
#include "opt/loop/loop_block.h"
#include "opt/optimizer_log.h"
#include "opt/pipeline/stage_scheduler.h"

namespace iroha {
namespace opt {
namespace pipeline {

IRegister *StateConditionInfo::GetCondRegisterByValue(int value) {
  vector<IRegister *> regs;
  for (auto it : cond_to_value_) {
    if (it.second == value) {
      regs.push_back(it.first);
    }
  }
  if (regs.size() == 0) {
    return nullptr;
  }
  // TODO: Needs some stable order.
  CHECK(regs.size() == 1);
  return regs[0];
}

InsnCondition::InsnCondition(loop::LoopBlock *lb, StageScheduler *ssch)
    : tab_(lb->GetTable()), lb_(lb), ssch_(ssch) {}

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
  SetMacroStageIndex();
  Dump(log);
  return true;
}

bool InsnCondition::IsCondReg(IRegister *reg) {
  return (cond_reg_info_.count(reg) == 1);
}

vector<IRegister *> InsnCondition::GetConditions() {
  vector<IRegister *> cond_regs;
  for (IRegister *reg : tab_->registers_) {
    if (cond_reg_info_.count(reg) == 1) {
      cond_regs.push_back(reg);
    }
  }
  return cond_regs;
}

int InsnCondition::GetConditionStateIndex(IRegister *cond_reg) {
  auto *cr = GetCondRegInfo(cond_reg);
  return cr->branch_mst_;
}

int InsnCondition::GetConditionLastUseStateIndex(IRegister *cond_reg) {
  auto *cr = GetCondRegInfo(cond_reg);
  return cr->last_use_mst_;
}

map<int, IRegister *> &InsnCondition::GetConditionRegStages(
    IRegister *cond_reg) {
  auto *cr = GetCondRegInfo(cond_reg);
  return cr->macro_stage_regs_;
}

IRegister *InsnCondition::GetFirstConditionRegStage(IRegister *cond_reg) {
  auto &m = GetConditionRegStages(cond_reg);
  return m.begin()->second;
}

void InsnCondition::Dump(OptimizerLog *log) {
  for (IState *br : branches_) {
    log->State(br) << "X";
  }
  for (auto &p : cond_value_info_) {
    ostream &os = log->State(p.first);
    StateConditionInfo *info = p.second;
    for (auto &q : info->cond_to_value_) {
      os << " " << q.first->GetId() << ":" << q.second;
    }
  }
  for (auto &q : cond_reg_info_) {
    ostream &os = log->Reg(q.first);
    ConditionRegInfo *info = q.second;
    int cond_st = lb_->GetIndexFromState(info->branch_st_);
    os << " *C" << cond_st << ":" << info->last_use_lst_ << " ";
    for (auto &v : info->values_) {
      os << v;
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
  map<IState *, set<int>> state_to_values;
  for (int i = 0; i < tr->target_states_.size(); ++i) {
    IState *next_st = tr->target_states_[i];
    set<IState *> reachable;
    CollectReachable(next_st, &reachable);
    for (IState *st : reachable) {
      state_to_values[st].insert(i);
    }
  }
  map<IState *, int> state_to_one_value;
  for (auto &p : state_to_values) {
    if (p.second.size() == 1) {
      int v = *(p.second.begin());
      state_to_one_value[p.first] = v;
    }
  }
  IRegister *cond_reg = tr->inputs_[0];
  for (auto &p : state_to_one_value) {
    IState *st = p.first;
    StateConditionInfo *info = cond_value_info_[st];
    if (info == nullptr) {
      info = new StateConditionInfo();
      cond_value_info_[st] = info;
    }
    // value at the state.
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
    StateConditionInfo *info = it->second;
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
    reg_info->last_use_lst_ = -1;
    cond_reg_info_[cond_reg] = reg_info;
  } else {
    reg_info = it->second;
  }
  return reg_info;
}

void InsnCondition::BuildConditionRegInfo() {
  for (auto &p : cond_value_info_) {
    StateConditionInfo *cvinfo = p.second;
    int st_index = lb_->GetIndexFromState(p.first);
    for (auto &r : cvinfo->cond_to_value_) {
      IRegister *cond_reg = r.first;
      ConditionRegInfo *reg_info = GetCondRegInfo(cond_reg);
      if (reg_info->last_use_lst_ < st_index) {
        reg_info->last_use_lst_ = st_index;
      }
      auto it = cvinfo->cond_to_value_.find(cond_reg);
      if (it != cvinfo->cond_to_value_.end()) {
        reg_info->values_.insert(it->second);
      }
    }
  }
}

void InsnCondition::SetMacroStageIndex() {
  for (auto &p : cond_reg_info_) {
    ConditionRegInfo *info = p.second;
    info->last_use_mst_ =
        ssch_->GetMacroStageFromLoopStateIndex(info->last_use_lst_);
    info->branch_mst_ = lb_->GetIndexFromState(info->branch_st_);
  }
}

pair<IRegister *, int> InsnCondition::GetInsnCondition(int nthst) {
  auto sts = lb_->GetStates();
  IState *st = sts[nthst];
  auto *info = cond_value_info_[st];
  if (info == nullptr) {
    return make_pair(nullptr, 0);
  }
  // Prefers 1, then tries 0.
  IRegister *cond_reg = info->GetCondRegisterByValue(1);
  if (cond_reg != nullptr) {
    return make_pair(cond_reg, 1);
  }
  return make_pair(info->GetCondRegisterByValue(0), 0);
}

}  // namespace pipeline
}  // namespace opt
}  // namespace iroha
