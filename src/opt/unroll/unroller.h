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
  Unroller(ITable *tab, LoopBlock *lb, int unroll_count);

  bool Unroll();

private:
  void UnrollOne();

  ITable *tab_;
  LoopBlock *lb_;
  int unroll_count_;
};

}  // namespace unroll
}  // namespace opt
}  // namespace iroha

#endif  // _opt_unroll_unroller_h_
