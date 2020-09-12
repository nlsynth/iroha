#include "opt/clean/unused_resource.h"

#include "iroha/i_design.h"
#include "iroha/resource_attr.h"

namespace iroha {
namespace opt {
namespace clean {

CleanUnusedResourcePass::~CleanUnusedResourcePass() {}

Pass *CleanUnusedResourcePass::Create() {
  return new CleanUnusedResourcePass();
}

bool CleanUnusedResourcePass::ApplyForDesign(IDesign *design) {
  return ApplyForAllModules("scan", design) &&
         ApplyForAllModules("collect", design);
}

bool CleanUnusedResourcePass::ApplyForTable(const string &key, ITable *table) {
  if (key == "scan") {
    return ScanTable(table);
  }
  if (key == "collect") {
    return CollectResource(table);
  }
  return true;
}

bool CleanUnusedResourcePass::ScanTable(ITable *table) {
  for (IState *st : table->states_) {
    for (IInsn *insn : st->insns_) {
      IResource *res = insn->GetResource();
      MarkResource(res);
    }
  }
  for (IResource *res : table->resources_) {
    if (ResourceAttr::IsExtAccessResource(res)) {
      MarkResource(res);
    }
  }
  return true;
}

void CleanUnusedResourcePass::MarkResource(IResource *res) {
  if (res == nullptr) {
    return;
  }
  if (used_resources_.find(res) != used_resources_.end()) {
    // already marked, so the parent and ancestors.
    return;
  }
  used_resources_.insert(res);
  MarkResource(res->GetParentResource());
  IArray *array = res->GetArray();
  if (array != nullptr) {
    IArrayImage *image = array->GetArrayImage();
    if (image != nullptr) {
      used_images_.insert(image);
    }
  }
}

bool CleanUnusedResourcePass::CollectResource(ITable *table) {
  vector<IResource *> new_resources;
  for (IResource *res : table->resources_) {
    if (used_resources_.find(res) != used_resources_.end()) {
      new_resources.push_back(res);
    }
  }
  table->resources_ = new_resources;
  vector<IArrayImage *> new_images;
  IDesign *design = table->GetModule()->GetDesign();
  for (IArrayImage *im : design->array_images_) {
    if (used_images_.find(im) != used_images_.end()) {
      new_images.push_back(im);
    }
  }
  design->array_images_ = new_images;
  return true;
}

}  // namespace clean
}  // namespace opt
}  // namespace iroha
