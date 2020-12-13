// -*- C++ -*-
#ifndef _opt_ssa_condition_value_range_h_
#define _opt_ssa_condition_value_range_h_

#include "opt/common.h"

namespace iroha {
namespace opt {
namespace ssa {

// Result for a query of range.
struct ConditionResult {
  IRegister *cond_reg;
  bool order01;
};

class PerCondition {
 public:
  IState *branch_st;
  // Mapping from a state to the value of condition reg.
  // (in case if only 1 value is determined).
  map<IState *, int> state_to_value;
};

class PerState {
 public:
  // condition regs which have only 1 possible value at this state.
  set<IRegister *> cond_regs;
};

struct CheckResult {
  bool ok;
  // s0 | next(s1, s2) | reg <= v0 - This assign is on_branch
  // s1 | next(s2)     | reg <= v1 - This assign is not on_branch
  // s2 | ...
  bool on_branch;
};

// ConditionValueRange tries to find a condition register c where PHI(r0, r1)
// can be rewritten to c0 ? r1 : r0 (or c0 ? r0 : r1).
// This can fail if there are loops and joins.
class ConditionValueRange {
 public:
  ConditionValueRange(ITable *table, OptimizerLog *opt_log);
  ~ConditionValueRange();

  void Build();
  ConditionResult Query(const vector<IRegister *> &regs);

 private:
  void BuildForBranch(IState *st, IInsn *insn);
  void PropagateConditionValue(PerCondition *pc, int nth, set<IState *> *sts);
  void BuildForStateWithValue(IRegister *cond, PerCondition *pc);
  void BuildRegToAssignState(set<IState *> reachable);
  PerState *GetPerState(IState *st, bool cr);
  void GetCandidateConditions(IRegister *reg, set<IRegister *> *cond_regs);
  CheckResult CheckConditionValue(IRegister *cond_reg, IRegister *reg,
                                  int value);
  void DumpToLog();
  IRegister *DeLocalizeRegister(IRegister *reg);

  ITable *table_;
  OptimizerLog *opt_log_;
  map<IRegister *, PerCondition *> per_cond_;
  map<IState *, PerState *> per_state_;
  map<IRegister *, IState *> reg_to_assign_state_;
};

}  // namespace ssa
}  // namespace opt
}  // namespace iroha

#endif  // _opt_ssa_condition_value_range_h_
