#include "opt/wire/resource_conflict_tracker.h"

namespace iroha {
namespace opt {
namespace wire {

ResourceConflictTracker::~ResourceConflictTracker() {
}

void ResourceConflictTracker::AddUsage(PathNode *node, bool had_conflict) {
}

}  // namespace wire
}  // namespace opt
}  // namespace iroha
