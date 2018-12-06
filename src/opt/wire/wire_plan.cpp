#include "opt/wire/wire_plan.h"

#include "iroha/stl_util.h"
#include "opt/wire/bb_wire_plan.h"
#include "opt/wire/data_path.h"
#include "opt/wire/resource_conflict_tracker.h"
#include "opt/wire/resource_entry.h"
#include "opt/wire/virtual_resource.h"

namespace iroha {
namespace opt {
namespace wire {


WirePlan::WirePlan(DataPathSet *dps, ResourceConflictTracker *conflict_tracker)
  : dps_(dps), conflict_tracker_(conflict_tracker) {
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
    num_replicas_[rent] = rent->GetNumReplicas();
  }
  auto &resources = vrs->GetVirtualResources();
  for (auto &p : resources) {
    VirtualResource *vres = p.second;
  }
}

void WirePlan::RestoreResources() {
  VirtualResourceSet *vrs = dps_->GetVirtualResourceSet();
  auto &entries = vrs->GetResourceEntries();
  for (auto &p : entries) {
    ResourceEntry *rent = p.second;
    rent->SetNumReplicas(num_replicas_[rent]);
  }
  auto &resources = vrs->GetVirtualResources();
  for (auto &p : resources) {
    VirtualResource *vres = p.second;
    vres->SetReplicaIndex(replica_indexes_[vres]);
  }
}

WirePlanSet::WirePlanSet(DataPathSet *dps) : dps_(dps) {
}

WirePlanSet::~WirePlanSet() {
  STLDeleteValues(&plans_);
}

void WirePlanSet::Save(ResourceConflictTracker *conflicts) {
  WirePlan *wp = new WirePlan(dps_, conflicts);
  wp->Save();
  plans_.push_back(wp);
}

void WirePlanSet::ApplyBest() {
}

WirePlan *WirePlanSet::GetLatestPlan() {
  if (plans_.size() == 0) {
    return nullptr;
  }
  return plans_[plans_.size() - 1];
}

}  // namespace wire
}  // namespace opt
}  // namespace iroha
