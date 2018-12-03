// -*- C++ -*-
#ifndef _opt_wire_resource_replica_assigner_
#define _opt_wire_resource_replica_assigner_

#include "opt/wire/common.h"

namespace iroha {
namespace opt {
namespace wire {

class ResourceReplicaAssigner {
public:
  ResourceReplicaAssigner(DataPathSet *dps);

  void Perform();

private:
  void PerformBB(BBDataPath *dp);
  void PerformNode(PathNode *node);

  DataPathSet *dps_;
};

}  // namespace wire
}  // namespace opt
}  // namespace iroha

#endif  // _opt_wire_resource_replica_assigner_
