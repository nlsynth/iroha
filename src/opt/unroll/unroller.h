// -*- C++ -*-
#ifndef _opt_unroll_unroller_h_
#define _opt_unroll_unroller_h_

#include "opt/common.h"

namespace iroha {
namespace opt {
namespace unroll {

class LoopBlock;

class Unroller {
public:
  Unroller(ITable *tab, LoopBlock *lb, int count);

  bool Unroll();

private:
  ITable *tab_;
  LoopBlock *lb_;
  int count_;
};

}  // namespace unroll
}  // namespace opt
}  // namespace iroha

#endif  // _opt_unroll_unroller_h_
