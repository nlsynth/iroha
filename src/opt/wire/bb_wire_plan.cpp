#include "opt/wire/bb_wire_plan.h"

#include "opt/wire/data_path.h"
#include "opt/wire/path_node.h"

namespace iroha {
namespace opt {
namespace wire {

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

}  // namespace wire
}  // namespace opt
}  // namespace iroha
