// -*- C++ -*-
#ifndef _opt_sched_plan_evaluator_h_
#define _opt_sched_plan_evaluator_h_

#include "opt/sched/common.h"

namespace iroha {
namespace opt {
namespace sched {

class PlanEvaluator {
public:
  // Returns the score of the plan (lower is better).
  // The score is an estimation of execution time from profiling data.
  long Evaluate(WirePlan *plan);

private:
  int GetBBSize(BBWirePlan *bb_plan);
};

}  // namespace sched
}  // namespace opt
}  // namespace iroha

#endif  // _opt_sched_plan_evaluator_h_
