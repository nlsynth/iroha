#include "opt/sched/resource_tracker.h"

#include "iroha/i_design.h"
#include "iroha/stl_util.h"
#include "opt/sched/path_node.h"
#include "opt/sched/resource_entry.h"
#include "opt/sched/virtual_resource.h"

namespace iroha {
namespace opt {
namespace sched {

// Per state resource usage.
class BBStateResourceUsage {
public:
  int GetUsage(ResourceEntry *re);
  void SetUsage(ResourceEntry *re, int count);

  map<ResourceEntry *, int> rent_usage_;
};

int BBStateResourceUsage::GetUsage(ResourceEntry *re) {
  return rent_usage_[re];
}

void BBStateResourceUsage::SetUsage(ResourceEntry *re, int count) {
  rent_usage_[re] = count;
}

BBResourceTracker::~BBResourceTracker() {
  STLDeleteSecondElements(&resource_usage_map_);
}

bool BBResourceTracker::CanUseResource(PathNode *node, int st_index) {
  VirtualResource *vres = node->GetVirtualResource();
  ResourceEntry *rent = vres->GetResourceEntry();
  int num_replicas = rent->GetNumReplicas();

  auto *u = GetResourceUsage(st_index);
  int usage = u->GetUsage(rent);
  if (usage < num_replicas) {
    return true;
  }
  return false;
}

void BBResourceTracker::AllocateResource(PathNode *node, int st_index) {
  auto *u = GetResourceUsage(st_index);
  VirtualResource *vres = node->GetVirtualResource();
  ResourceEntry *rent = vres->GetResourceEntry();
  int usage = u->GetUsage(rent);
  u->SetUsage(rent, usage + 1);
}

BBStateResourceUsage *BBResourceTracker::GetResourceUsage(int st_index) {
  auto *u = resource_usage_map_[st_index];
  if (u == nullptr) {
    u = new BBStateResourceUsage();
    resource_usage_map_[st_index] = u;
  }
  return u;
}

}  // namespace sched
}  // namespace opt
}  // namespace iroha
