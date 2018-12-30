// -*- C++ -*-
#ifndef _opt_sched_plan_evaluator_h_
#define _opt_sched_plan_evaluator_h_

#include "opt/sched/common.h"

namespace iroha {
namespace opt {
namespace sched {

class PlanEvaluator {
public:
  PlanEvaluator(DataPathSet *dps);

  long Evaluate(WirePlan *plan);

private:
  int GetBBSize(BBWirePlan *bb_plan);

  DataPathSet *dps_;
};

}  // namespace sched
}  // namespace opt
}  // namespace iroha

#endif  // _opt_sched_plan_evaluator_h_
