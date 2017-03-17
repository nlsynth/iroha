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

bool CleanUnusedResourcePhase::ApplyForDesign(IDesign *design) {
  return ApplyForAllModules("scan", design) &&
    ApplyForAllModules("collect", design);
}

bool CleanUnusedResourcePhase::ApplyForTable(const string &key, ITable *table) {
  if (key == "scan") {
    return ScanTable(table);
  }
  if (key == "collect") {
    return CollectResource(table);
  }
  return true;
}

bool CleanUnusedResourcePhase::ScanTable(ITable *table) {
  for (IState *st : table->states_) {
    for (IInsn *insn : st->insns_) {
      IResource *res = insn->GetResource();
      used_resources_.insert(res);
      IResource *parent = res->GetParentResource();
      if (parent != nullptr) {
	used_resources_.insert(parent);
      }
    }
  }
  return true;
}

bool CleanUnusedResourcePhase::CollectResource(ITable *table) {
  vector<IResource *> new_resources;
  for (IResource *res : table->resources_) {
    if (used_resources_.find(res) != used_resources_.end()) {
      new_resources.push_back(res);
    }
  }
  table->resources_ = new_resources;
  return true;
}

}  // namespace clean
}  // namespace opt
}  // namespace iroha
