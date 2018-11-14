#include "opt/wire/resource_entry.h"

namespace iroha {
namespace opt {
namespace wire {

ResourceEntry::ResourceEntry(IResource *res) : res_(res) {
}

IResource *ResourceEntry::GetResource() {
  return res_;
}

}  // namespace wire
}  // namespace opt
}  // namespace iroha
