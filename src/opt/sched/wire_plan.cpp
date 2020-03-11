#include "opt/sched/wire_plan.h"

#include "iroha/stl_util.h"
#include "opt/sched/bb_wire_plan.h"
#include "opt/sched/bb_data_path.h"
#include "opt/sched/data_path_set.h"
#include "opt/sched/plan_evaluator.h"
#include "opt/sched/resource_conflict_tracker.h"
#include "opt/sched/resource_entry.h"
#include "opt/sched/virtual_resource.h"

namespace iroha {
namespace opt {
namespace sched {

void AllocationPlan::Restore() {
  for (auto &p : num_replicas_) {
    ResourceEntry *rent = p.first;
    int num = p.second;
    rent->SetNumReplicas(num);
  }
}

WirePlan::WirePlan(DataPathSet *dps, ResourceConflictTracker *conflict_tracker)
  : dps_(dps), conflict_tracker_(conflict_tracker), score_(0) {
  auto &paths = dps->GetPaths();
  for (auto p : paths) {
    int bb_id = p.first;
    BBDataPath *bb_path = p.second;
    bb_plans_[bb_id] = new BBWirePlan(bb_path);
  }
}

WirePlan::~WirePlan() {
  STLDeleteSecondElements(&bb_plans_);
}

void WirePlan::Save() {
  for (auto p : bb_plans_) {
    BBWirePlan *plan = p.second;
    plan->Save();
  }
  SaveResources();
}

void WirePlan::Restore() {
  for (auto p : bb_plans_) {
    BBWirePlan *plan = p.second;
    plan->Restore();
  }
  RestoreResources();
}

ResourceConflictTracker *WirePlan::GetConflictTracker() {
  return conflict_tracker_.get();
}

void WirePlan::SaveResources() {
  VirtualResourceSet *vrs = dps_->GetVirtualResourceSet();
  auto &entries = vrs->GetResourceEntries();
  for (auto &p : entries) {
    ResourceEntry *rent = p.second;
    allocation_plan_.num_replicas_[rent] = rent->GetNumReplicas();
  }
}

void WirePlan::RestoreResources() {
  VirtualResourceSet *vrs = dps_->GetVirtualResourceSet();
  auto &entries = vrs->GetResourceEntries();
  for (auto &p : entries) {
    ResourceEntry *rent = p.second;
    rent->SetNumReplicas(allocation_plan_.num_replicas_[rent]);
  }
  auto &resources = vrs->GetVirtualResources();
  for (auto &p : resources) {
    VirtualResource *vres = p.second;
    vres->SetReplicaIndex(replica_indexes_[vres]);
  }
}

void WirePlan::SetScore(long score) {
  score_ = score;
}

long WirePlan::GetScore() {
  return score_;
}

map<int, BBWirePlan *> &WirePlan::GetBBWirePlans() {
  return bb_plans_;
}

AllocationPlan &WirePlan::GetAllocationPlan() {
  return allocation_plan_;
}

WirePlanSet::WirePlanSet() {
}

WirePlanSet::~WirePlanSet() {
  STLDeleteValues(&plans_);
}

void WirePlanSet::SaveCurrentSchedulingPlan(DataPathSet *dps,
					    ResourceConflictTracker *conflicts,
					    PlanEvaluator *ev) {
  WirePlan *wp = new WirePlan(dps, conflicts);
  wp->Save();
  wp->SetScore(ev->Evaluate(wp));
  plans_.push_back(wp);
}

void WirePlanSet::ApplyBest() {
  WirePlan *best_plan = nullptr;
  long best_score = 0;
  for (auto *plan : plans_) {
    long score = plan->GetScore();
    if (score < best_score || (best_score == 0)) {
      best_score = score;
      best_plan = plan;
    }
  }
  if (best_plan != nullptr) {
    best_plan->Restore();
  }
}

WirePlan *WirePlanSet::GetLatestPlan(int nth) {
  int index = plans_.size() - 1 - nth;
  if (index < 0) {
    return nullptr;
  }
  return plans_[index];
}

}  // namespace sched
}  // namespace opt
}  // namespace iroha
