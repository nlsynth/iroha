// -*- C++ -*-
#ifndef _opt_bb_set_h_
#define _opt_bb_set_h_

#include "iroha/common.h"

namespace iroha {
namespace opt {

class DebugAnnotation;

class BB {
public:
  vector<IState *> states_;
};

class BBSet {
public:
  ~BBSet();
  static BBSet *Create(ITable *table, DebugAnnotation *annotation);
  void Annotate(DebugAnnotation *annotation);

  vector<BB *> bbs_;
};

}  // namespace opt
}  // namespace iroha

#endif  // _opt_bb_set_h_
