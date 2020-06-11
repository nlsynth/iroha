// -*- C++ -*-
#ifndef _opt_sched_virtual_resource_h_
#define _opt_sched_virtual_resource_h_

#include "opt/sched/common.h"

namespace iroha {
namespace opt {
namespace sched {

// Allocated for per insn.
// Multiple VirtualResource-s share a ResourceEntry.
class VirtualResource {
public:
  VirtualResource(VirtualResourceSet *vrset, IInsn *insn);

  ResourceEntry *GetResourceEntry();
  void SetResourceEntry(ResourceEntry *re);
  IInsn *GetInsn();
  int GetReplicaIndex();
  void SetReplicaIndex(int replica_index);

private:
  VirtualResourceSet *vrset_;
  IInsn *insn_;
  ResourceEntry *res_;
  int replica_index_;
};

}  // namespace sched
}  // namespace opt
}  // namespace iroha

#endif  // _opt_sched_virtual_resource_h_
