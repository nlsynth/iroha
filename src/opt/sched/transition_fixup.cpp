#include "opt/sched/transition_fixup.h"

#include "opt/sched/bb_data_path.h"
#include "opt/sched/path_node.h"
#include "opt/sched/data_path_set.h"

namespace iroha {
namespace opt {
namespace sched {

TransitionFixup::TransitionFixup(DataPathSet *dps) : dps_(dps) {
}

void TransitionFixup::Perform() {
  auto &paths = dps_->GetBBPaths();
  for (auto &p : paths) {
    ProcessBB(p.second);
  }
}

void TransitionFixup::ProcessBB(BBDataPath *bbp) {
  int max_tr = -1;
  int max_non_tr = -1;
  auto &nodes = bbp->GetNodes();
  for (auto &n : nodes) {
    PathNode *node = n.second;
    int idx = node->GetFinalStIndex();
    if (node->IsTransition()) {
      if (idx > max_tr) {
	max_tr = idx;
      }
    } else {
      if (idx > max_non_tr) {
	max_non_tr = idx;
      }
    }
  }
  if (max_tr <= max_non_tr || max_non_tr <= 0) {
    return;
  }
  for (auto &n : nodes) {
    PathNode *node = n.second;
    if (!node->IsTransition()) {
      continue;
    }
    int idx = node->GetFinalStIndex();
    if (idx == max_tr) {
      node->SetFinalStIndex(max_non_tr);
    } else if (idx >= max_non_tr) {
      node->SetFinalStIndex(-1);
      // WIP. kill it.
    }
  }
}

}  // namespace sched
}  // namespace opt
}  // namespace iroha
