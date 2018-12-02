#include "opt/wire/wire_plan.h"

#include "iroha/stl_util.h"
#include "opt/wire/bb_wire_plan.h"
#include "opt/wire/data_path.h"
#include "opt/wire/resource_conflict_tracker.h"

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
}

void WirePlan::Restore() {
  for (auto p : bb_plans_) {
    BBWirePlan *plan = p.second;
    plan->Restore();
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

}  // namespace wire
}  // namespace opt
}  // namespace iroha
