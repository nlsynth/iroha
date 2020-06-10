#include "opt/sched/explorer.h"

#include "design/resource_attr.h"
#include "opt/sched/resource_conflict_tracker.h"
#include "opt/sched/resource_entry.h"
#include "opt/sched/wire_plan.h"

namespace iroha {
namespace opt {
namespace sched {

Explorer::Explorer(WirePlanSet *wps) : wps_(wps) {
}

void Explorer::SetInitialAllocation() {
  // Do nothing.
}

bool Explorer::MaySetNextAllocationPlan() {
  // Fixes too many resource uses if current plan has them.
  bool has_update = MayResolveTooManyResourceUses();
  if (has_update) {
    WirePlan *wp = wps_->GetLatestPlan(0);
    wp->SetScore(0);
    return true;
  }
  // Otherwise proceed to standard exploration.
  return ExploreNewPlan();
}

bool Explorer::MayResolveTooManyResourceUses() {
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
    // Avoids too many usage of same resource.
    // (Use very conservative value for now)
    const int kMaxUsage = 2;
    if (replicas * kMaxUsage < usage_count) {
      has_update = true;
      re->SetNumReplicas((usage_count + kMaxUsage - 1) / kMaxUsage);
    }
  }
  return has_update;
}

bool Explorer::ExploreNewPlan() {
  if (!HadSufficientImprovement()) {
    return false;
  }
  WirePlan *last = wps_->GetLatestPlan(0);
  return SetNewPlan(last);
}

bool Explorer::HadSufficientImprovement() {
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
  return true;
}

bool Explorer::SetNewPlan(WirePlan *wp) {
  AllocationPlan alloc_plan = wp->GetAllocationPlan();
  ResourceConflictTracker *conflict_tracker = wp->GetConflictTracker();
  auto &usage = conflict_tracker->GetUsageCount();
  int max_usage_rate = 0;
  // Finds the most used resource.
  for (auto p : alloc_plan.num_replicas_) {
    ResourceEntry *re = p.first;
    int num_replicas = p.second;
    int rate = GetUsageRate(re, num_replicas, usage);
    if (rate > max_usage_rate) {
      max_usage_rate = rate;
    }
  }
  if (max_usage_rate <= 1) {
    return false;
  }
  // Allocates more replicas.
  for (auto p : alloc_plan.num_replicas_) {
    ResourceEntry *re = p.first;
    int num_replicas = p.second;
    int rate = GetUsageRate(re, num_replicas, usage);
    if (rate == max_usage_rate) {
      p.second++;
    }
  }
  alloc_plan.Restore();
  return true;
}

int Explorer::GetUsageRate(ResourceEntry *re, int num_replicas,
			   const map<ResourceEntry *, int> &usage) {
  if (num_replicas == 0) {
    return 0;
  }
  int u = 0;
  auto it = usage.find(re);
  if (it != usage.end()) {
    u = it->second;
  }
  return u / num_replicas;
}

}  // namespace sched
}  // namespace opt
}  // namespace iroha
