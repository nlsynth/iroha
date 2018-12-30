// -*- C++ -*-
#ifndef _opt_sched_resource_conflict_tracker_
#define _opt_sched_resource_conflict_tracker_

#include "opt/sched/common.h"

namespace iroha {
namespace opt {
namespace sched {

// Per table object to track the number of resource conflicts during
// one scheduling iteration.
class ResourceConflictTracker {
public:
  ~ResourceConflictTracker();

  void AddUsage(PathNode *node, bool had_conflict);
  map<ResourceEntry *, int> &GetUsageCount();
  map<ResourceEntry *, int> &GetConflictCount();

  void Dump(DebugAnnotation *an);

private:
  map<ResourceEntry *, int> usage_count_;
  map<ResourceEntry *, int> conflict_count_;
};

}  // namespace sched
}  // namespace opt
}  // namespace iroha

#endif  // _opt_sched_resource_conflict_tracker_
