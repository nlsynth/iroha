#include "opt/dominator_tree_builder.h"

#include "iroha/stl_util.h"
#include "iroha/logging.h"
#include "opt/dominator_tree.h"
#include "opt/bb_set.h"

namespace iroha {
namespace opt {

DominatorTreeBuilder::DominatorTreeBuilder(BBSet *bset,
					   DebugAnnotation *an)
  : bset_(bset), annotation_(an) {
}

DominatorTreeBuilder::~DominatorTreeBuilder() {
  STLDeleteSecondElements(&dom_info_);
}

DominatorTree *DominatorTreeBuilder::Create() {
  for (BB *bb : bset_->bbs_) {
    dom_info_[bb] = new DominatorInfo(bb);
  }
  CalculateDominator();
  CalculateFrontier();
  DominatorTree *dt = new DominatorTree();
  for (auto it : dom_info_) {
    set<BB *> frontiers;
    for (auto *di : it.second->frontiers_) {
      frontiers.insert(di->bb_);
    }
    BBSet::SortBBs(frontiers, &dt->frontiers_[it.first]);
  }
  return dt;
}

void DominatorTreeBuilder::CalculateDominator() {
  for (auto it : dom_info_) {
    DominatorInfo *di = it.second;
    BB *bb = it.first;
    // Sets initial dominator values.
    if (bb == bset_->initial_bb_) {
      di->dominators_.insert(di);
    } else {
      for (auto jt : dom_info_) {
	di->dominators_.insert(jt.second);
      }
    }
    // Sets pred blocks' info.
    for (BB *next_bb : di->bb_->next_bbs_) {
      auto *next_di = dom_info_[next_bb];
      next_di->pred_.insert(di);
    }
  }
  bool changed;
  do {
    changed = false;
    for (auto it : dom_info_) {
      if (it.first == bset_->initial_bb_) {
        continue;
      }
      // U(all pred blocks) + itself.
      DominatorInfo *di = it.second;
      CHECK(di->pred_.size() > 0);
      set<DominatorInfo *> cur = (*(di->pred_.begin()))->dominators_;
      for (DominatorInfo *pred_di : di->pred_) {
        set<DominatorInfo *> tmp;
        Union(cur, pred_di->dominators_, &tmp);
        cur = tmp;
      }
      cur.insert(di);
      // update
      if (cur != di->dominators_) {
        changed = true;
        di->dominators_ = cur;
      }
    }
  } while (changed);
}

void DominatorTreeBuilder::Union(set<DominatorInfo *> &s1,
				 set<DominatorInfo *> &s2,
				 set<DominatorInfo *> *d) {
  for (auto *di1 : s1) {
    if (s2.find(di1) != s2.end()) {
      d->insert(di1);
    }
  }
}

void DominatorTreeBuilder::CalculateFrontier() {
  for (auto it : dom_info_) {
    DominatorInfo *cur_di = it.second;
    for (BB *bb : cur_di->bb_->next_bbs_) {
     DominatorInfo *next_di = dom_info_[bb];
     for (DominatorInfo *di : cur_di->dominators_) {
       if (next_di->dominators_.find(di) == next_di->dominators_.end()) {
         di->frontiers_.insert(next_di);
       }
     }
   }
  }
}

}  // namespace opt
}  // namespace iroha
