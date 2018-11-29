#include "opt/wire/resource_conflict_tracker.h"

#include "iroha/i_design.h"
#include "opt/debug_annotation.h"
#include "opt/wire/path_node.h"
#include "opt/wire/resource_entry.h"
#include "opt/wire/virtual_resource.h"

namespace iroha {
namespace opt {
namespace wire {

ResourceConflictTracker::~ResourceConflictTracker() {
}

void ResourceConflictTracker::AddUsage(PathNode *node, bool had_conflict) {
  if (node->IsTransition()) {
    // Ignore.
    return;
  }
  VirtualResource *vres = node->GetVirtualResource();
  ResourceEntry *rent = vres->GetResourceEntry();
  usage_count_[rent]++;
  if (had_conflict) {
    conflict_count_[rent]++;
  }
}

void ResourceConflictTracker::Dump(DebugAnnotation *an) {
  ostream &os = an->GetDumpStream();
  os << "Resource conflicts\n"
     << "# class id replica count conflict\n";
  for (auto p : usage_count_) {
    ResourceEntry *rent = p.first;
    int count = p.second;
    int conflict = conflict_count_[rent];
    IResource *ires = rent->GetResource();
    os << ires->GetClass()->GetName() << " "
       << ires->GetId() << " "
       << rent->GetNumReplicas() << " "
       << count << " "
       << conflict
       << "\n";
  }
}

}  // namespace wire
}  // namespace opt
}  // namespace iroha
