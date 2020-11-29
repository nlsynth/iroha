// -*- C++ -*-
#ifndef _opt_ssa_condition_value_range_h_
#define _opt_ssa_condition_value_range_h_

#include "opt/common.h"

namespace iroha {
namespace opt {
namespace ssa {

struct ConditionResult {
  IRegister *cond_reg;
  bool order01;
};

class PerCondition {
 public:
  IState *st;
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

  ITable *table_;
  map<IRegister *, PerCondition *> per_cond_;
};

}  // namespace ssa
}  // namespace opt
}  // namespace iroha

#endif  // _opt_ssa_condition_value_range_h_
