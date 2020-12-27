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
  IRegister *GetInitialCounterValue();
  IRegister *GetCounterRegister();
  vector<IState *> &GetStates();
  IState *GetEntryAssignState();
  IState *GetExitState();
  IState *GetCompareState();
  IInsn *GetCompareInsn();
  IInsn *GetBranchInsn();
  IInsn *GetIncrementInsn();

  void Annotate(OptimizerLog *log);

 private:
  pair<IState *, IInsn *> FindInitialAssign();
  IRegister *FindInitialValue(IInsn *insn);
  IState *CheckInitialAssign(IState *st, IInsn *insn);
  pair<IState *, IInsn *> FindCompareInsn(IState *initial_assign_st);
  IInsn *CompareResult(IInsn *insn);
  IState *FindTransition(IState *compare_st, IInsn *compare_insn);
  void CollectLoopStates(IState *exit_st, IState *compare_st);
  void FindIncrementInsn();
  void BuildConstRegMap();

  ITable *tab_;
  IRegister *counter_reg_;
  IRegister *counter_initial_reg_;
  IState *initial_assign_st_;
  IState *compare_st_;
  IInsn *compare_insn_;
  IInsn *branch_insn_;
  IState *exit_st_;
  IInsn *increment_insn_;
  int loop_count_;
  vector<IState *> states_;
  set<IRegister *> regs_with_const_;
};

}  // namespace loop
}  // namespace opt
}  // namespace iroha

#endif  // _opt_loop_loop_block_h_
