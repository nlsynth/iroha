// -*- C++ -*-
#ifndef _opt_wire_plan_evaluator_h_
#define _opt_wire_plan_evaluator_h_

#include "opt/wire/common.h"

namespace iroha {
namespace opt {
namespace wire {

class PlanEvaluator {
public:
  PlanEvaluator(DataPathSet *dps);

  long Evaluate(WirePlan *plan);

private:
  DataPathSet *dps_;
};

}  // namespace wire
}  // namespace opt
}  // namespace iroha

#endif  // _opt_wire_plan_evaluator_h_
