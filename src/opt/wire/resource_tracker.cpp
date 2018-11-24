#include "opt/wire/resource_tracker.h"

#include "iroha/i_design.h"
#include "opt/wire/path_node.h"

namespace iroha {
namespace opt {
namespace wire {

BBResourceTracker::~BBResourceTracker() {
}

bool BBResourceTracker::CanUseResource(PathNode *node, int st_index) {
  auto key = std::make_tuple(node->GetInsn()->GetResource(), st_index);
  auto it = resource_slots_.find(key);
  if (it == resource_slots_.end()) {
    return true;
  }
  return false;
}

void BBResourceTracker::AllocateResource(PathNode *node, int st_index) {
  auto key = std::make_tuple(node->GetInsn()->GetResource(), st_index);
  resource_slots_.insert(key);
}

}  // namespace wire
}  // namespace opt
}  // namespace iroha
