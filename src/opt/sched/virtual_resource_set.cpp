#include "opt/sched/virtual_resource_set.h"

#include "iroha/i_design.h"
#include "iroha/stl_util.h"
#include "opt/sched/resource_entry.h"
#include "opt/sched/virtual_resource.h"

namespace iroha {
namespace opt {
namespace sched {

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

}  // namespace sched
}  // namespace opt
}  // namespace iroha
