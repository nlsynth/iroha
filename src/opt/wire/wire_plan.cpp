#include "opt/wire/wire_plan.h"

#include "iroha/stl_util.h"
#include "opt/wire/resource_conflict_tracker.h"

namespace iroha {
namespace opt {
namespace wire {


WirePlan::WirePlan(DataPathSet *dps, ResourceConflictTracker *conflict_tracker)
  : dps_(dps), conflict_tracker_(conflict_tracker) {
}

WirePlan::~WirePlan() {
}

void WirePlan::Save() {
}

void WirePlan::Restore() {
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
