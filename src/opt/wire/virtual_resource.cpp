#include "opt/wire/virtual_resource.h"

#include "iroha/stl_util.h"

namespace iroha {
namespace opt {
namespace wire {

VirtualResource::VirtualResource(VirtualResourceSet *vrset, IResource *res)
  : vrset_(vrset), res_(res) {
}

VirtualResourceSet::VirtualResourceSet(ITable *tab) : tab_(tab) {
}

VirtualResourceSet::~VirtualResourceSet() {
  STLDeleteSecondElements(&raw_resources_);
}

VirtualResource *VirtualResourceSet::GetOriginalResource(IResource *res) {
  auto it = raw_resources_.find(res);
  if (it != raw_resources_.end()) {
    return it->second;
  }
  VirtualResource *vres = new VirtualResource(this, res);
  raw_resources_[res] = vres;
  return vres;
}

}  // namespace wire
}  // namespace opt
}  // namespace iroha
