// -*- C++ -*-
#ifndef _opt_bb_set_h_
#define _opt_bb_set_h_

#include "opt/common.h"

namespace iroha {
namespace opt {

class DebugAnnotation;

class BB {
public:
  BB();

  vector<IState *> states_;
  set<BB *> next_bbs_;
  set<BB *> prev_bbs_;
};

class BBSet {
public:
  BBSet(ITable *table);
  ~BBSet();

  static BBSet *Create(ITable *table, DebugAnnotation *annotation);

  void Annotate(DebugAnnotation *annotation);
  ITable *GetTable() const;

  vector<BB *> bbs_;
  map<IState *, BB *> state_to_bb_;

private:
  ITable *table_;
};

}  // namespace opt
}  // namespace iroha

#endif  // _opt_bb_set_h_
