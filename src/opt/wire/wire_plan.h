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
  void SetScore(long score);
  long GetScore();

  ResourceConflictTracker *GetConflictTracker();
  map<int, BBWirePlan *> &GetBBWirePlans();

private:
  void SaveResources();
  void RestoreResources();

  DataPathSet *dps_;
  std::unique_ptr<ResourceConflictTracker> conflict_tracker_;
  // Smaller is better, but 0 indicates a problem like congestion.
  long score_;
  map<int, BBWirePlan *> bb_plans_;
  map<ResourceEntry *, int> num_replicas_;
  map<VirtualResource *, int> replica_indexes_;
};

class WirePlanSet {
public:
  WirePlanSet(DataPathSet *dps, PlanEvaluator *ev);
  ~WirePlanSet();

  void Save(ResourceConflictTracker *conflicts);
  void ApplyBest();

  WirePlan *GetLatestPlan();

private:
  DataPathSet *dps_;
  PlanEvaluator *ev_;
  vector<WirePlan *> plans_;
};

}  // namespace wire
}  // namespace opt
}  // namespace iroha

#endif  // _opt_wire_wire_plan_h_
