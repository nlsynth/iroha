#include "opt/sched/plan_evaluator.h"

#include "iroha/i_design.h"
#include "opt/bb_set.h"
#include "opt/sched/bb_wire_plan.h"
#include "opt/sched/data_path.h"
#include "opt/sched/path_node.h"
#include "opt/sched/wire_plan.h"

namespace iroha {
namespace opt {
namespace sched {

PlanEvaluator::PlanEvaluator(DataPathSet *dps) : dps_(dps) {
}

long PlanEvaluator::Evaluate(WirePlan *plan) {
  long score = 0;
  auto &bb_plans = plan->GetBBWirePlans();
  for (auto p : bb_plans) {
    BBWirePlan *bb_plan = p.second;
    int size = GetBBSize(bb_plan);
    BB *bb = bb_plan->GetBBDataPath()->GetBB();
    const IProfile &profile = bb->states_[0]->GetProfile();
    if (!profile.valid_) {
      continue;
    }
    score += size * (1 << profile.normalized_count_);
  }
  return score + 1;
}

int PlanEvaluator::GetBBSize(BBWirePlan *bb_plan) {
  int max = 0;
  auto &indexes = bb_plan->GetStIndexes();
  for (auto p : indexes) {
    PathNode *node = p.first;
    if (node->IsTransition()) {
      continue;
    }
    int index = p.second;
    if (index > max) {
      max = index;
    }
  }
  return max;
}

}  // namespace sched
}  // namespace opt
}  // namespace iroha
