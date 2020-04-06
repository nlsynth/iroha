// -*- C++ -*-
#ifndef _opt_unroll_state_copier_h_
#define _opt_unroll_state_copier_h_

#include "opt/common.h"

namespace iroha {
namespace opt {
namespace unroll {

class LoopBlock;

class StateCopier {
public:
  StateCopier(ITable *tab, LoopBlock *lb);

  void Copy();

private:
  void CopyState(IState *st);

  ITable *tab_;
  LoopBlock *lb_;
  IState *exit_state_;

  vector<IState *> new_states_;
  // origin to new.
  map<IInsn *, IInsn *> insn_copy_map_;
  map<IState *, IState *> state_copy_map_;
};

}  // namespace unroll
}  // namespace opt
}  // namespace iroha

#endif  // _opt_unroll_state_copier_h_
