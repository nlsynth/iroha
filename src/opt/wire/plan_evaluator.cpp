#include "opt/wire/plan_evaluator.h"

namespace iroha {
namespace opt {
namespace wire {

PlanEvaluator::PlanEvaluator(DataPathSet *dps) : dps_(dps) {
}

long PlanEvaluator::Evaluate(WirePlan *plan) {
  return 1;
}

}  // namespace wire
}  // namespace opt
}  // namespace iroha
