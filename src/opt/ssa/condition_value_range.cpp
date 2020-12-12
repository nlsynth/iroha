#include "opt/ssa/condition_value_range.h"

#include "design/design_util.h"
#include "iroha/i_design.h"
#include "iroha/stl_util.h"
#include "opt/opt_util.h"
#include "opt/optimizer_log.h"

namespace iroha {
namespace opt {
namespace ssa {

ConditionValueRange::ConditionValueRange(ITable *table, OptimizerLog *opt_log)
    : table_(table), opt_log_(opt_log) {}

ConditionValueRange::~ConditionValueRange() {
  STLDeleteSecondElements(&per_cond_);
  STLDeleteSecondElements(&per_state_);
}

void ConditionValueRange::Build() {
  set<IState *> reachable;
  OptUtil::CollectReachableStates(table_, &reachable);
  vector<pair<IState *, IInsn *>> branches;
  set<IRegister *> cond_regs;
  for (IState *st : reachable) {
    IInsn *tr_insn = DesignUtil::FindTransitionInsn(st);
    if (tr_insn == nullptr) {
      continue;
    }
    if (tr_insn->target_states_.size() < 2) {
      continue;
    }
    branches.push_back(make_pair(st, tr_insn));
    cond_regs.insert(tr_insn->inputs_[0]);
  }
  if (branches.size() != cond_regs.size()) {
    // cond regs are reused by multiple branch insns. gives up.
    return;
  }
  for (auto &p : branches) {
    BuildForBranch(p.first, p.second);
  }
  for (auto &pc : per_cond_) {
    BuildForStateWithValue(pc.first, pc.second);
  }
  BuildRegToAssignState(reachable);
  DumpToLog();
}

ConditionResult ConditionValueRange::Query(const vector<IRegister *> &regs) {
  ConditionResult res;
  res.cond_reg = nullptr;
  if (regs.size() != 2) {
    return res;
  }
  set<IRegister *> c0;
  GetCandidateConditions(regs[0], &c0);
  set<IRegister *> c1;
  GetCandidateConditions(regs[1], &c1);
  for (IRegister *cond_reg : table_->registers_) {
    if (c0.find(cond_reg) == c0.end()) {
      continue;
    }
    if (c1.find(cond_reg) == c1.end()) {
      continue;
    }
    if (CheckConditionValue(cond_reg, regs[0], 0) &&
        CheckConditionValue(cond_reg, regs[1], 1)) {
      res.cond_reg = cond_reg;
      res.order01 = true;
    }
    if (CheckConditionValue(cond_reg, regs[0], 1) &&
        CheckConditionValue(cond_reg, regs[1], 0)) {
      res.cond_reg = cond_reg;
      res.order01 = false;
    }
  }
  if (res.cond_reg != nullptr && res.cond_reg->IsStateLocal()) {
    res.cond_reg = DeLocalizeRegister(res.cond_reg);
  }
  return res;
}

IRegister *ConditionValueRange::DeLocalizeRegister(IRegister *reg) {
  if (!reg->IsStateLocal()) {
    return reg;
  }
  IState *st = reg_to_assign_state_[reg];
  IResource *assign = DesignUtil::FindAssignResource(table_);
  // Finds already existing non state local.
  for (IInsn *insn : st->insns_) {
    if (insn->GetResource() == assign && insn->inputs_[0] == reg) {
      IRegister *r = insn->outputs_[0];
      if (r->IsNormal()) {
        return r;
      }
    }
  }
  // NOTE: Not allocating a new register for now.
  return nullptr;
}

void ConditionValueRange::BuildForBranch(IState *st, IInsn *insn) {
  IRegister *cond = insn->inputs_[0];
  PerCondition *pc = new PerCondition();
  pc->branch_st = st;
  per_cond_[cond] = pc;
  map<IState *, set<int>> values;
  for (int i = 0; i < insn->target_states_.size(); ++i) {
    set<IState *> sts;
    PropagateConditionValue(pc, i, &sts);
    for (IState *st : sts) {
      values[st].insert(i);
    }
  }
  for (auto &p : values) {
    if (p.second.size() == 1) {
      int v = *(p.second.begin());
      IState *s = p.first;
      pc->state_to_value[s] = v;
    }
  }
}

void ConditionValueRange::PropagateConditionValue(PerCondition *pc, int nth,
                                                  set<IState *> *sts) {
  set<IState *> seen;
  set<IState *> frontier;
  IInsn *initial_tr_insn = DesignUtil::FindTransitionInsn(pc->branch_st);
  frontier.insert(initial_tr_insn->target_states_[nth]);
  while (frontier.size() > 0) {
    IState *st = *(frontier.begin());
    frontier.erase(st);
    if (st == pc->branch_st) {
      continue;
    }
    if (seen.find(st) != seen.end()) {
      continue;
    }
    seen.insert(st);
    IInsn *tr_insn = DesignUtil::FindTransitionInsn(st);
    if (tr_insn == nullptr) {
      continue;
    }
    sts->insert(st);
    for (IState *next_st : tr_insn->target_states_) {
      frontier.insert(next_st);
    }
  }
}

void ConditionValueRange::BuildForStateWithValue(IRegister *cond,
                                                 PerCondition *pc) {
  for (auto &sv : pc->state_to_value) {
    PerState *ps = GetPerState(sv.first, true);
    ps->cond_regs.insert(cond);
  }
}

PerState *ConditionValueRange::GetPerState(IState *st, bool cr) {
  auto it = per_state_.find(st);
  if (it != per_state_.end()) {
    return it->second;
  }
  if (!cr) {
    return nullptr;
  }
  PerState *ps = new PerState();
  per_state_[st] = ps;
  return ps;
}

void ConditionValueRange::BuildRegToAssignState(set<IState *> reachable) {
  map<IRegister *, set<IState *>> raw;
  for (IState *st : reachable) {
    for (IInsn *insn : st->insns_) {
      for (IRegister *oreg : insn->outputs_) {
        raw[oreg].insert(st);
      }
    }
  }
  for (auto &p : raw) {
    auto &s = p.second;
    if (s.size() == 1) {
      reg_to_assign_state_[p.first] = *(s.begin());
    }
  }
}

void ConditionValueRange::GetCandidateConditions(IRegister *reg,
                                                 set<IRegister *> *cond_regs) {
  IState *st = reg_to_assign_state_[reg];
  PerState *ps = GetPerState(st, false);
  if (ps == nullptr) {
    // This state is a branch state.
    IInsn *tr_insn = DesignUtil::FindTransitionInsn(st);
    cond_regs->insert(tr_insn->inputs_[0]);
    return;
  }
  for (IRegister *cr : ps->cond_regs) {
    cond_regs->insert(cr);
  }
}

bool ConditionValueRange::CheckConditionValue(IRegister *cond_reg,
                                              IRegister *reg, int value) {
  IState *st = reg_to_assign_state_[reg];
  PerCondition *pc = per_cond_[cond_reg];
  if (pc->branch_st == st) {
    return true;
  }
  auto it = pc->state_to_value.find(st);
  if (it != pc->state_to_value.end()) {
    if (it->second == value) {
      return true;
    }
  }
  return false;
}

void ConditionValueRange::DumpToLog() {
  for (auto &p : per_state_) {
    PerState *ps = p.second;
    if (ps->cond_regs.size() == 0) {
      continue;
    }
    IState *st = p.first;
    ostream &os = opt_log_->State(p.first);
    os << "[";
    for (IRegister *reg : ps->cond_regs) {
      PerCondition *pc = per_cond_[reg];
      int value = pc->state_to_value[st];
      os << "r" << reg->GetId() << ":" << value << " ";
    }
    os << "]";
  }
}

}  // namespace ssa
}  // namespace opt
}  // namespace iroha
