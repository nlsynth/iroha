// -*- C++ -*-
#ifndef _opt_dominator_tree_h_
#define _opt_dominator_tree_h_

#include "opt/common.h"

namespace iroha {
namespace opt {

class DominatorTree {
public:
  static DominatorTree *Create(BBSet *bset,
			       DebugAnnotation *an);
  void GetFrontier(BB *bb, vector<BB *> *s);

  map<BB *, vector<BB *>> frontiers_;
};

}  // namespace opt
}  // namespace iroha

#endif  // _opt_dominator_tree_h_
