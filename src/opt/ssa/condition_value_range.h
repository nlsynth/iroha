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

class ConditionValueRange {
 public:
  ConditionValueRange(ITable *table);

  void Build();
  ConditionResult Query(const vector<IRegister *> &regs);

 private:
  ITable *table_;
};

}  // namespace ssa
}  // namespace opt
}  // namespace iroha

#endif  // _opt_ssa_condition_value_range_h_
