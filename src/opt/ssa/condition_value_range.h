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
  bool CheckConditionValue(IRegister *cond_reg, IRegister *reg, int value);
  void DumpToLog();

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
