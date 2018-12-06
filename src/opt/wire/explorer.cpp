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
  return MayResolveCongestion();
}

bool Explorer::MayResolveCongestion() {
  WirePlan *wp = wps_->GetLatestPlan();
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

}  // namespace wire
}  // namespace opt
}  // namespace iroha
