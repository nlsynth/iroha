#include "opt/wire/wire_plan.h"

#include "iroha/stl_util.h"

namespace iroha {
namespace opt {
namespace wire {


WirePlan::WirePlan(DataPathSet *dps) : dps_(dps) {
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

void WirePlanSet::Save() {
  WirePlan *wp = new WirePlan(dps_);
  wp->Save();
  plans_.push_back(wp);
}

void WirePlanSet::ApplyBest() {
}

}  // namespace wire
}  // namespace opt
}  // namespace iroha
