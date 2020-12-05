#include "opt/ssa/condition_value_range.h"

#include "design/design_util.h"
#include "iroha/i_design.h"
#include "iroha/stl_util.h"
#include "opt/opt_util.h"

namespace iroha {
namespace opt {
namespace ssa {

ConditionValueRange::ConditionValueRange(ITable *table) : table_(table) {}

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
    BuildForTransition(p.first, p.second);
  }
  for (auto &pc : per_cond_) {
    BuildForState(pc.first, pc.second);
  }
  BuildRegToAssignState(reachable);
}

ConditionResult ConditionValueRange::Query(const vector<IRegister *> &regs) {
  ConditionResult res;
  res.cond_reg = nullptr;
  return res;
}

void ConditionValueRange::BuildForTransition(IState *st, IInsn *insn) {
  IRegister *cond = insn->inputs_[0];
  PerCondition *pc = new PerCondition();
  pc->st = st;
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
      pc->value[s] = v;
    }
  }
}

void ConditionValueRange::PropagateConditionValue(PerCondition *pc, int nth,
                                                  set<IState *> *sts) {
  set<IState *> seen;
  set<IState *> frontier;
  IInsn *initial_tr_insn = DesignUtil::FindTransitionInsn(pc->st);
  frontier.insert(initial_tr_insn->target_states_[nth]);
  while (frontier.size() > 0) {
    IState *st = *(frontier.begin());
    frontier.erase(st);
    if (st == pc->st) {
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
    for (IState *next_st : tr_insn->target_states_) {
      frontier.insert(next_st);
    }
  }
}

void ConditionValueRange::BuildForState(IRegister *cond, PerCondition *pc) {
  for (auto &sv : pc->value) {
    PerState *ps = GetPerState(sv.first);
    ps->regs.insert(cond);
  }
}

PerState *ConditionValueRange::GetPerState(IState *st) {
  auto it = per_state_.find(st);
  if (it != per_state_.end()) {
    return it->second;
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

}  // namespace ssa
}  // namespace opt
}  // namespace iroha
