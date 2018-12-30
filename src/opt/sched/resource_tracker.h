// -*- C++ -*-
#ifndef _opt_sched_resource_tracker_h_
#define _opt_sched_resource_tracker_h_

#include "opt/sched/common.h"

namespace iroha {
namespace opt {
namespace sched {

class BBStateResourceUsage;

// Per BB object used by Scheduler to track resource usage.
class BBResourceTracker {
public:
  ~BBResourceTracker();
  bool CanUseResource(PathNode *node, int st_index);
  void AllocateResource(PathNode *node, int st_index);

private:
  BBStateResourceUsage *GetResourceUsage(int st_index);

  map<int, BBStateResourceUsage *> resource_usage_map_;
};

}  // namespace sched
}  // namespace opt
}  // namespace iroha

#endif  // _opt_sched_resource_tracker_h_
