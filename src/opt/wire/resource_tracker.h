// -*- C++ -*-
#ifndef _opt_wire_resource_tracker_h_
#define _opt_wire_resource_tracker_h_

#include "opt/wire/common.h"

namespace iroha {
namespace opt {
namespace wire {

class BBStateResourceUsage;

// Per BB object used by Scheduler to track resource usage.
class BBResourceTracker {
public:
  ~BBResourceTracker();
  bool CanUseResource(PathNode *node, int st_index);
  void AllocateResource(PathNode *node, int st_index);

private:
  BBStateResourceUsage *GetResourceUsage(int st_index);

  set<std::tuple<IResource *, int> > resource_slots_;
  map<int, BBStateResourceUsage *> resource_usage_map_;
};

}  // namespace wire
}  // namespace opt
}  // namespace iroha

#endif  // _opt_wire_resource_tracker_h_
