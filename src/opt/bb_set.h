// -*- C++ -*-
#ifndef _opt_bb_set_h_
#define _opt_bb_set_h_

#include "iroha/common.h"

namespace iroha {
namespace opt {

class BB {
public:
  vector<IState *> states_;
};

class BBSet {
public:
  static BBSet *Create(ITable *table);

  vector<BB *> bbs_;
};

}  // namespace opt
}  // namespace iroha

#endif  // _opt_bb_set_h_
