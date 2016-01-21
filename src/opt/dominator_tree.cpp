#include "opt/dominator_tree.h"

#include "opt/dominator_tree_builder.h"

namespace iroha {
namespace opt {

DominatorTree *DominatorTree::Create(BBSet *bset,
				     DebugAnnotation *an) {
  DominatorTreeBuilder builder(bset, an);
  return builder.Create();
}

void DominatorTree::GetFrontier(BB *bb, vector<BB *> *s) {
  *s = frontiers_[bb];
}

}  // namespace opt
}  // namespace iroha
