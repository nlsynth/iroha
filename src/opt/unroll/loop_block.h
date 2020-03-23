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

private:
  void FindInitialAssign(IInsn *insn);

  ITable *tab_;
  IRegister *reg_;
  IInsn *initial_assign_;
};

}  // namespace unroll
}  // namespace opt
}  // namespace iroha

#endif  // _opt_unroll_loop_block_h_
