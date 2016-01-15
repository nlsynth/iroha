// -*- C++ -*-
#ifndef _opt_bb_set_h_
#define _opt_bb_set_h_

#include "iroha/common.h"

namespace iroha {
namespace opt {

class DebugAnnotation;

class BB {
public:
  BB();
  vector<IState *> states_;
};

class BBSet {
public:
  BBSet(ITable *table);
  ~BBSet();

  static BBSet *Create(ITable *table, DebugAnnotation *annotation);

  void Annotate(DebugAnnotation *annotation);
  ITable *GetTable() const;

  vector<BB *> bbs_;

private:
  ITable *table_;
};

}  // namespace opt
}  // namespace iroha

#endif  // _opt_bb_set_h_
