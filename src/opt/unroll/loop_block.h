// -*- C++ -*-
#ifndef _opt_unroll_loop_block_h_
#define _opt_unroll_loop_block_h_

#include "opt/common.h"

namespace iroha {
namespace opt {
namespace unroll {

class LoopBlock {
public:
  LoopBlock(ITable *tab, IRegister *reg);

  bool Build();

  int GetLoopCount();

private:
  void FindInitialAssign(IState *st, IInsn *insn);
  IInsn *CompareResult(IInsn *insn);
  IState *FindTransition(IState *compare_st, IInsn *compare_insn);
  void CollectLoopStates(IState *exit_st, IState *compare_st);

  ITable *tab_;
  IRegister *reg_;
  IState *initial_assign_st_;
  int loop_count_;
  vector<IState *> states_;
};

}  // namespace unroll
}  // namespace opt
}  // namespace iroha

#endif  // _opt_unroll_loop_block_h_
