// -*- C++ -*-
#ifndef _opt_loop_loop_block_h_
#define _opt_loop_loop_block_h_

#include "opt/common.h"

namespace iroha {
namespace opt {
namespace loop {

// Loop states *.
//   (Sa) Sets initial value
// * (Sb) num > count then ->Sc else -> Se
// * (Sc..) ..
// * (Sd) count += 1, ->Sb
//   (Se) ..
class LoopBlock {
 public:
  LoopBlock(ITable *tab, IRegister *reg);

  bool Build();

  int GetLoopCount();
  ITable *GetTable();
  IRegister *GetRegister();
  vector<IState *> &GetStates();
  IState *GetEntryAssignState();
  IState *GetExitState();
  IState *GetCompareState();
  IInsn *GetCompareInsn();
  IInsn *GetBranchInsn();

  void Annotate(OptimizerLog *log);

 private:
  void FindInitialAssign(IState *st, IInsn *insn);
  IInsn *CompareResult(IInsn *insn);
  IState *FindTransition(IState *compare_st, IInsn *compare_insn);
  void CollectLoopStates(IState *exit_st, IState *compare_st);

  ITable *tab_;
  IRegister *reg_;
  IState *initial_assign_st_;
  IState *compare_st_;
  IInsn *compare_insn_;
  IInsn *branch_insn_;
  IState *exit_st_;
  int loop_count_;
  vector<IState *> states_;
};

}  // namespace loop
}  // namespace opt
}  // namespace iroha

#endif  // _opt_loop_loop_block_h_
