#include "opt/wire/virtual_resource.h"

#include "iroha/stl_util.h"

namespace iroha {
namespace opt {
namespace wire {

VirtualResource::VirtualResource(VirtualResourceSet *vrset, IInsn *insn)
  : vrset_(vrset), insn_(insn) {
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

}  // namespace wire
}  // namespace opt
}  // namespace iroha
