// -*- C++ -*-
#ifndef _opt_unroll_unroller_h_
#define _opt_unroll_unroller_h_

#include "opt/common.h"

namespace iroha {
namespace opt {
namespace unroll {

class LoopBlock;
class StateCopier;

class Unroller {
public:
  Unroller(ITable *tab, LoopBlock *lb, int unroll_count);

  bool Unroll();

private:
  void UnrollOne(bool is_head);
  void Reconnect();

  ITable *tab_;
  LoopBlock *lb_;
  int unroll_count_;
  std::unique_ptr<vector<StateCopier *> > copiers_;
};

}  // namespace unroll
}  // namespace opt
}  // namespace iroha

#endif  // _opt_unroll_unroller_h_
