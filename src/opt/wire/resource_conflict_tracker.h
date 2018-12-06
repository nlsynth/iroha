// -*- C++ -*-
#ifndef _opt_wire_resource_conflict_tracker_
#define _opt_wire_resource_conflict_tracker_

#include "opt/wire/common.h"

namespace iroha {
namespace opt {
namespace wire {

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

}  // namespace wire
}  // namespace opt
}  // namespace iroha

#endif  // _opt_wire_resource_conflict_tracker_
