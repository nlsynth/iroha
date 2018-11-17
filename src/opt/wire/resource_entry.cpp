#include "opt/wire/resource_entry.h"

namespace iroha {
namespace opt {
namespace wire {

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

}  // namespace wire
}  // namespace opt
}  // namespace iroha
