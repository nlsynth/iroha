// -*- C++ -*-
#ifndef _opt_sched_resource_entry_h_
#define _opt_sched_resource_entry_h_

#include "opt/sched/common.h"

namespace iroha {
namespace opt {
namespace sched {

// Represents a resource type during this phase.
// Multiple VirtualResource can points to this.
// Owned by VirtualResourceSet.
class ResourceEntry {
public:
  ResourceEntry(IResource *res);

  IResource *GetResource();
  int GetNumReplicas();
  void SetNumReplicas(int num_replicas);
  void PrepareReplicas();
  IResource *GetNthReplica(int nth);

private:
  // Template resource.
  IResource *res_;
  // replicas_[0] = res_
  vector<IResource *> replicas_;
  int num_replicas_;
};

}  // namespace sched
}  // namespace opt
}  // namespace iroha

#endif  // _opt_sched_resource_entry_h_
