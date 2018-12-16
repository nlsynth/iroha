#include "opt/wire/explorer.h"

#include "design/resource_attr.h"
#include "opt/wire/resource_conflict_tracker.h"
#include "opt/wire/resource_entry.h"
#include "opt/wire/wire_plan.h"

namespace iroha {
namespace opt {
namespace wire {

Explorer::Explorer(WirePlanSet *wps) : wps_(wps) {
}

void Explorer::SetInitialAllocation() {
  // Do nothing.
}

bool Explorer::MaySetNextAllocationPlan() {
  bool congested = MayResolveCongestion();
  if (congested) {
    WirePlan *wp = wps_->GetLatestPlan(0);
    wp->SetScore(0);
    return true;
  }
  return ExploreNewPlan();
}

bool Explorer::MayResolveCongestion() {
  WirePlan *wp = wps_->GetLatestPlan(0);
  if (wp == nullptr) {
    return false;
  }
  bool has_update = false;
  ResourceConflictTracker *conflict_tracker = wp->GetConflictTracker();
  auto &usage = conflict_tracker->GetUsageCount();
  for (auto p : usage) {
    ResourceEntry *re = p.first;
    IResource *ires = re->GetResource();
    if (!ResourceAttr::IsDuplicatableResource(ires)) {
      continue;
    }
    int usage_count = p.second;
    int replicas = re->GetNumReplicas();
    const int kMaxUsage = 2;
    if (replicas * kMaxUsage < usage_count) {
      has_update = true;
      re->SetNumReplicas((usage_count + kMaxUsage - 1) / kMaxUsage);
    }
  }
  return has_update;
}

bool Explorer::ExploreNewPlan() {
  WirePlan *last = wps_->GetLatestPlan(0);
  WirePlan *second_last = wps_->GetLatestPlan(1);
  if (second_last != nullptr) {
    long last_score = last->GetScore();
    long second_last_score = second_last->GetScore();
    if (last_score * 4 < second_last_score * 5) {
      // Improvement is less than 25%.
      return false;
    }
  }
  return SetNewPlan(last);
}

bool Explorer::SetNewPlan(WirePlan *wp) {
  return false;
}

}  // namespace wire
}  // namespace opt
}  // namespace iroha
