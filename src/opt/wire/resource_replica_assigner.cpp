#include "opt/wire/resource_replica_assigner.h"

#include "opt/wire/data_path.h"
#include "opt/wire/path_node.h"
#include "opt/wire/resource_entry.h"
#include "opt/wire/virtual_resource.h"

namespace iroha {
namespace opt {
namespace wire {

ResourceReplicaAssigner::ResourceReplicaAssigner(DataPathSet *dps) : dps_(dps) {
}

void ResourceReplicaAssigner::Perform() {
  auto &paths = dps_->GetPaths();
  for (auto p : paths) {
    BBDataPath *dp = p.second;
    PerformBB(dp);
  }
}

void ResourceReplicaAssigner::PerformBB(BBDataPath *dp) {
  auto &nodes = dp->GetNodes();
  for (auto p : nodes) {
    PathNode *pn = p.second;
    PerformNode(pn);
  }
}

void ResourceReplicaAssigner::PerformNode(PathNode *node) {
  VirtualResource *vres = node->GetVirtualResource();
  ResourceEntry *rent = vres->GetResourceEntry();
  auto &usage = usage_count_[rent];
  if (usage.size() == 0) {
    usage.resize(rent->GetNumReplicas());
  }
  // Finds the least used replica.
  int min_index = 0;
  int min_usage = 1000;
  for (int i = 0; i < usage.size(); ++i) {
    if (usage[i] < min_usage) {
      min_usage = usage[i];
      min_index = i;
    }
  }
  usage[min_index]++;
  vres->SetReplicaIndex(min_index);
}

}  // namespace wire
}  // namespace opt
}  // namespace iroha
