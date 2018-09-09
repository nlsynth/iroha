#include "opt/wire/resource_share.h"

#include "iroha/i_design.h"
#include "iroha/stl_util.h"

namespace iroha {
namespace opt {
namespace wire {

class ResourceEntry {
public:
  ResourceEntry() : ires_(nullptr) {
  }
  IResource *ires_;
  set<IInsn *> using_insns_;
};

ResourceShare::ResourceShare(ITable *tab) : tab_(tab) {
}

ResourceShare::~ResourceShare() {
  STLDeleteSecondElements(&entries_);
}

void ResourceShare::Scan() {
  // Populates resource entries_.
  for (IResource *res : tab_->resources_) {
    ResourceEntry *re = new ResourceEntry;
    re->ires_ = res;
    entries_[res] = re;
  }
  // Count the number of insns for each resource.
  for (IState *st : tab_->states_) {
    for (IInsn *insn : st->insns_) {
      ResourceEntry *re = entries_[insn->GetResource()];
      re->using_insns_.insert(insn);
    }
  }
}

}  // namespace wire
}  // namespace opt
}  // namespace iroha
