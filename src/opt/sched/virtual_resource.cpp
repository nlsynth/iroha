#include "opt/sched/virtual_resource.h"

namespace iroha {
namespace opt {
namespace sched {

VirtualResource::VirtualResource(VirtualResourceSet *vrset, IInsn *insn)
  : vrset_(vrset), insn_(insn), res_(nullptr), replica_index_(0) {
}


ResourceEntry *VirtualResource::GetResourceEntry() {
  return res_;
}

void VirtualResource::SetResourceEntry(ResourceEntry *re) {
  res_ = re;
}

IInsn *VirtualResource::GetInsn() {
  return insn_;
}

int VirtualResource::GetReplicaIndex() {
  return replica_index_;
}

void VirtualResource::SetReplicaIndex(int replica_index) {
  // replica_index < res_->GetNumReplicas()
  replica_index_ = replica_index;
}

}  // namespace sched
}  // namespace opt
}  // namespace iroha
