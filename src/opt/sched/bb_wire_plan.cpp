#include "opt/sched/bb_wire_plan.h"

#include "opt/sched/data_path.h"
#include "opt/sched/path_node.h"

namespace iroha {
namespace opt {
namespace sched {

BBWirePlan::BBWirePlan(BBDataPath *dp) : dp_(dp) {
}

BBWirePlan::~BBWirePlan() {
}

void BBWirePlan::Save() {
  auto &nodes = dp_->GetNodes();
  for (auto p : nodes) {
    PathNode *pn = p.second;
    st_indexes_[pn] = pn->GetFinalStIndex();
  }
}

void BBWirePlan::Restore() {
  for (auto p : st_indexes_) {
    PathNode *pn = p.first;
    int index = p.second;
    pn->SetFinalStIndex(index);
  }
}

BBDataPath *BBWirePlan::GetBBDataPath() {
  return dp_;
}

map<PathNode *, int> &BBWirePlan::GetStIndexes() {
  return st_indexes_;
}

}  // namespace sched
}  // namespace opt
}  // namespace iroha
