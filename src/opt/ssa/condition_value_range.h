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
  IState *st;
  // Mapping from a state to the value of condition reg.
  // (in case if only 1 value is determined).
  map<IState *, int> value;
};

class PerState {
 public:
  // condition regs which have only 1 possible value at this state.
  set<IRegister *> regs;
};

class ConditionValueRange {
 public:
  ConditionValueRange(ITable *table);
  ~ConditionValueRange();

  void Build();
  ConditionResult Query(const vector<IRegister *> &regs);

 private:
  void BuildForTransition(IState *st, IInsn *insn);
  void PropagateConditionValue(PerCondition *pc, int nth, set<IState *> *sts);
  void BuildForState(IRegister *cond, PerCondition *pc);
  void BuildRegToAssignState(set<IState *> reachable);
  PerState *GetPerState(IState *st);

  ITable *table_;
  map<IRegister *, PerCondition *> per_cond_;
  map<IState *, PerState *> per_state_;
  map<IRegister *, IState *> reg_to_assign_state_;
};

}  // namespace ssa
}  // namespace opt
}  // namespace iroha

#endif  // _opt_ssa_condition_value_range_h_
