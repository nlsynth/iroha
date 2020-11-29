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
  for (int i = 0; i < insn->target_states_.size(); ++i) {
    set<IState *> sts;
    PropagateConditionValue(pc, i, &sts);
  }
}

void ConditionValueRange::PropagateConditionValue(PerCondition *pc, int n,
                                                  set<IState *> *sts) {}

}  // namespace ssa
}  // namespace opt
}  // namespace iroha