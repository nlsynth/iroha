#include "opt/sched/resource_entry.h"

#include "design/design_tool.h"

namespace iroha {
namespace opt {
namespace sched {

ResourceEntry::ResourceEntry(IResource *res) : res_(res), num_replicas_(1) {
}

IResource *ResourceEntry::GetResource() {
  return res_;
}

int ResourceEntry::GetNumReplicas() {
  return num_replicas_;
}

void ResourceEntry::SetNumReplicas(int num_replicas) {
  num_replicas_ = num_replicas;
}

void ResourceEntry::PrepareReplicas() {
  replicas_.push_back(res_);
  for (int i = 1; i < num_replicas_; ++i) {
    replicas_.push_back(DesignTool::CopySimpleResource(res_));
  }
}

IResource *ResourceEntry::GetNthReplica(int nth) {
  return replicas_[nth];
}

}  // namespace sched
}  // namespace opt
}  // namespace iroha
