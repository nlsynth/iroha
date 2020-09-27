// -*- C++ -*-
#ifndef _opt_unroll_state_copier_h_
#define _opt_unroll_state_copier_h_

#include "opt/common.h"

namespace iroha {
namespace opt {
namespace unroll {

class StateCopier {
 public:
  StateCopier(ITable *tab, loop::LoopBlock *lb, bool is_head);

  void Copy();
  IState *GetInitialState();
  IState *GetContinueState();

 private:
  void CopyState(IState *st);

  ITable *tab_;
  loop::LoopBlock *lb_;
  bool is_head_;
  IState *continue_st_;

  vector<IState *> new_states_;
  // origin to new.
  map<IInsn *, IInsn *> insn_copy_map_;
  map<IState *, IState *> state_copy_map_;
};

}  // namespace unroll
}  // namespace opt
}  // namespace iroha

#endif  // _opt_unroll_state_copier_h_
