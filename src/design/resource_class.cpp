#include "design/resource_class.h"

#include "iroha/i_design.h"
#include "iroha/logging.h"

namespace iroha {

class IDesign;

static void InstallResource(IDesign *design, const string &name,
			    bool is_exclusive) {
  IResourceClass *klass = new IResourceClass(name, is_exclusive, design);
  design->resource_classes_.push_back(klass);
}

void InstallResourceClasses(IDesign *design) {
  InstallResource(design, resource::kSet, false);
  InstallResource(design, resource::kTransition, true);
}

IResourceClass *GetTransitionResourceClassFromDesign(IDesign *design) {
  for (auto *rc : design->resource_classes_) {
    if (rc->GetName() == resource::kTransition) {
      return rc;
    }
  }
  CHECK(false) << "Transition resource class is not installed?";
  return nullptr;
}

}  // namespace iroha
