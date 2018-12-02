#include "opt/wire/virtual_resource.h"

#include "iroha/i_design.h"
#include "iroha/stl_util.h"
#include "opt/wire/resource_entry.h"

namespace iroha {
namespace opt {
namespace wire {

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

VirtualResourceSet::VirtualResourceSet(ITable *tab) : tab_(tab) {
}

VirtualResourceSet::~VirtualResourceSet() {
  STLDeleteSecondElements(&raw_resources_);
}

VirtualResource *VirtualResourceSet::GetFromInsn(IInsn *insn) {
  auto it = raw_resources_.find(insn);
  if (it != raw_resources_.end()) {
    return it->second;
  }
  VirtualResource *vres = new VirtualResource(this, insn);
  raw_resources_[insn] = vres;
  return vres;
}

void VirtualResourceSet::BuildDefaultBinding() {
  // Binds existing each IResource to a new ResourceEntry.
  // We might consider to merge similar or compatible resources.
  for (auto &p : raw_resources_) {
    VirtualResource *vr = p.second;
    IResource *res = vr->GetInsn()->GetResource();
    auto it = default_resource_entries_.find(res);
    ResourceEntry *re = nullptr;
    if (it == default_resource_entries_.end()) {
      re = new ResourceEntry(res);
      default_resource_entries_[res] = re;
    } else {
      re = it->second;
    }
    vr->SetResourceEntry(re);
  }
}

void VirtualResourceSet::PrepareReplicas() {
  for (auto &p : default_resource_entries_) {
    auto *rent = p.second;
    rent->PrepareReplicas();
  }
}

map<IResource *, ResourceEntry *> &VirtualResourceSet::GetResourceEntries() {
  return default_resource_entries_;
}

map<IInsn *, VirtualResource *> &VirtualResourceSet::GetVirtualResources() {
  return raw_resources_;
}

}  // namespace wire
}  // namespace opt
}  // namespace iroha
