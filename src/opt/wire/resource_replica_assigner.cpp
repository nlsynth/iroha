#include "opt/wire/resource_replica_assigner.h"

#include "opt/wire/data_path.h"

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
}

}  // namespace wire
}  // namespace opt
}  // namespace iroha
