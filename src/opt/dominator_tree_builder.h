// -*- C++ -*-
#ifndef _opt_dominator_tree_builder_h_
#define _opt_dominator_tree_builder_h_

#include "opt/common.h"

namespace iroha {
namespace opt {

class DominatorTreeBuilder {
public:
  DominatorTreeBuilder(BBSet *bset,
		       DebugAnnotation *an);
  ~DominatorTreeBuilder();

  DominatorTree *Create();

private:
  class DominatorInfo {
  public:
    DominatorInfo(BB *bb) : bb_(bb) {
    }
    BB *bb_;
    set<DominatorInfo *> dominators_;
    set<DominatorInfo *> frontiers_;
    set<DominatorInfo *> pred_;
  };
  void CalculateDominator();
  void Union(set<DominatorInfo *> &s1,
             set<DominatorInfo *> &s2,
             set<DominatorInfo *> *d);
  void CalculateFrontier();

  ITable *table_;
  BBSet *bset_;
  DebugAnnotation *annotation_;
  map<BB *, DominatorInfo *> dom_info_;
};

}  // namespace opt
}  // namespace iroha

#endif  // _opt_dominator_tree_builder_h_
