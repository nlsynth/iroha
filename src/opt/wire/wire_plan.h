// -*- C++ -*-
#ifndef _opt_wire_wire_plan_h_
#define _opt_wire_wire_plan_h_

#include "opt/wire/common.h"

namespace iroha {
namespace opt {
namespace wire {

// A snapshot of scheduling and allocation result.
class WirePlan {
public:
  WirePlan(DataPathSet *dps, ResourceConflictTracker *conflict_tracker);
  ~WirePlan();

  void Save();
  void Restore();

private:
  DataPathSet *dps_;
  std::unique_ptr<ResourceConflictTracker> conflict_tracker_;
};

class WirePlanSet {
public:
  WirePlanSet(DataPathSet *dps);
  ~WirePlanSet();

  void Save(ResourceConflictTracker *conflicts);
  void ApplyBest();

private:
  DataPathSet *dps_;
  vector<WirePlan *> plans_;
};

}  // namespace wire
}  // namespace opt
}  // namespace iroha

#endif  // _opt_wire_wire_plan_h_
