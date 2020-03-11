// -*- C++ -*-
//
// BB stands for Basic Block.
//
#ifndef _opt_bb_set_h_
#define _opt_bb_set_h_

#include "opt/common.h"

namespace iroha {
namespace opt {

class BB {
public:
  BB();

  vector<IState *> states_;
  int bb_id_;
  set<BB *> next_bbs_;
  set<BB *> prev_bbs_;
};

class BBSet {
public:
  BBSet(ITable *table);
  ~BBSet();

  static BBSet *Create(ITable *table, DebugAnnotation *annotation);
  static BBSet *CreateWip(ITable *table, bool splitMultiCycle,
			  DebugAnnotation *annotation);
  static void SortBBs(const set<BB *> &input, vector<BB *> *sorted);

  void Annotate(DebugAnnotation *annotation);
  ITable *GetTable() const;

  vector<BB *> bbs_;
  BB *initial_bb_;
  map<IState *, BB *> state_to_bb_;

private:
  ITable *table_;
};

}  // namespace opt
}  // namespace iroha

#endif  // _opt_bb_set_h_
