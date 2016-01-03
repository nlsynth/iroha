#include "design/resource_class.h"

#include "design/object_pool.h"
#include "iroha/i_design.h"
#include "iroha/logging.h"
#include "iroha/resource_class.h"

namespace iroha {

class IDesign;

static void InstallResource(IDesign *design, const string &name,
			    bool is_exclusive) {
  IResourceClass *klass = new IResourceClass(name, is_exclusive, design);
  design->resource_classes_.push_back(klass);
  ObjectPool *pool = design->GetObjectPool();
  pool->resource_classes_.Add(klass);
}

void InstallResourceClasses(IDesign *design) {
  InstallResource(design, resource::kSet, false);
  InstallResource(design, resource::kPrint, false);
  InstallResource(design, resource::kAssert, false);
  InstallResource(design, resource::kChannelWrite, true);
  InstallResource(design, resource::kChannelRead, false);
  InstallResource(design, resource::kSubModuleTaskCall, true);
  InstallResource(design, resource::kSubModuleTask, true);
  InstallResource(design, resource::kTransition, true);
  InstallResource(design, resource::kEmbedded, true);
  InstallResource(design, resource::kExtInput, true);
  InstallResource(design, resource::kExtOutput, true);
  InstallResource(design, resource::kArray, true);
  InstallResource(design, resource::kGt, true);
  InstallResource(design, resource::kAdd, true);
  InstallResource(design, resource::kSub, true);
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
