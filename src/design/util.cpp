#include "design/util.h"

#include "iroha/i_design.h"

namespace iroha {

IModule *DesignUtil::GetRootModule(const IDesign *design) {
  IModule *root = nullptr;
  for (auto *mod : design->modules_) {
    if (mod->GetParentModule() == nullptr) {
      if (root == nullptr) {
	root = mod;
      } else {
	// Don't allow multiple roots.
	return nullptr;
      }
    }
  }
  return root;
}

vector<IModule *> DesignUtil::GetChildModules(const IModule *parent) {
  const IDesign *design = parent->GetDesign();
  vector<IModule *> v;
  for (auto *mod : design->modules_) {
    if (mod->GetParentModule() == parent) {
      v.push_back(mod);
    }
  }
  return v;
}

}  // namespace iroha
