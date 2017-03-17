#include "opt/clean/unused_resource.h"

#include "iroha/i_design.h"

namespace iroha {
namespace opt {
namespace clean {

CleanUnusedResourcePhase::~CleanUnusedResourcePhase() {
}

Phase *CleanUnusedResourcePhase::Create() {
  return new CleanUnusedResourcePhase();
}

bool CleanUnusedResourcePhase::ApplyForTable(const string &key, ITable *table) {
  set<IResource *> used_resources;
  for (IState *st : table->states_) {
    for (IInsn *insn : st->insns_) {
      used_resources.insert(insn->GetResource());
    }
  }
  vector<IResource *> new_resources;
  for (IResource *res : table->resources_) {
    if (used_resources.find(res) != used_resources.end()) {
      new_resources.push_back(res);
    }
  }
  table->resources_ = new_resources;
  return true;
}

}  // namespace clean
}  // namespace opt
}  // namespace iroha
