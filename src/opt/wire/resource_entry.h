// -*- C++ -*-
#ifndef _opt_wire_resource_entry_h_
#define _opt_wire_resource_entry_h_

#include "opt/wire/common.h"

namespace iroha {
namespace opt {
namespace wire {

// Represents a resource type during this phase.
// Multiple VirtualResource can points to this.
class ResourceEntry {
public:
  ResourceEntry(IResource *res);

  IResource *GetResource();
  int GetNumReplicas();
  void SetNumReplicas(int num_replicas);

private:
  // Template resource.
  IResource *res_;
  int num_replicas_;
};

}  // namespace wire
}  // namespace opt
}  // namespace iroha

#endif  // _opt_wire_resource_entry_h_
