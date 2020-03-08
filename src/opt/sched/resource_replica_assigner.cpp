#include "opt/sched/resource_replica_assigner.h"

#include "opt/sched/data_path.h"
#include "opt/sched/data_path_set.h"
#include "opt/sched/path_node.h"
#include "opt/sched/resource_entry.h"
#include "opt/sched/virtual_resource.h"

namespace iroha {
namespace opt {
namespace sched {

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
  auto &ps = per_state_[make_tuple(node->GetBBDataPath(), node->GetFinalStIndex())];
  if (ps.size() == 0) {
    ps.resize(rent->GetNumReplicas());
  }
  // Finds the globally least used and replica (and unused in this state).
  // TODO: Aren't there possibilities of assigning same replica too many times?
  // Maybe I should sort states before this.
  int min_index = 0;
  int min_usage = 1000;
  for (int i = 0; i < usage.size(); ++i) {
    if (ps[i]) {
      continue;
    }
    if (usage[i] < min_usage) {
      min_usage = usage[i];
      min_index = i;
    }
  }
  usage[min_index]++;
  ps[min_index] = true;
  vres->SetReplicaIndex(min_index);
}

}  // namespace sched
}  // namespace opt
}  // namespace iroha
