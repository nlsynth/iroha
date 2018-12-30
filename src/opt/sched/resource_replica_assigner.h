// -*- C++ -*-
#ifndef _opt_sched_resource_replica_assigner_
#define _opt_sched_resource_replica_assigner_

#include "opt/sched/common.h"

namespace iroha {
namespace opt {
namespace sched {

class ResourceReplicaAssigner {
public:
  ResourceReplicaAssigner(DataPathSet *dps);

  void Perform();

private:
  void PerformBB(BBDataPath *dp);
  void PerformNode(PathNode *node);

  DataPathSet *dps_;
  map<ResourceEntry *, vector<int> > usage_count_;
  map<tuple<BBDataPath *, int>, vector<bool> > per_state_;
};

}  // namespace sched
}  // namespace opt
}  // namespace iroha

#endif  // _opt_sched_resource_replica_assigner_
