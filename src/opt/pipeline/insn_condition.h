// -*- C++ -*-
#ifndef _opt_pipeline_insn_condition_h_
#define _opt_pipeline_insn_condition_h_

#include "opt/common.h"

namespace iroha {
namespace opt {
namespace pipeline {

class StageScheduler;

// per state.
struct InsnConditionValueInfo {
  map<IRegister *, int> cond_to_value_;
  vector<IInsn *> insns_;
  IRegister *GetCondRegister(int v);
};

struct ConditionRegInfo {
  IState *branch_st_;
  int branch_mst_;
  int last_use_lst_;
  int last_use_mst_;
  map<int, IRegister *> macro_stage_regs_;

  set<int> values_;
};

// Finds conditional executions in a pipeline.
class InsnCondition {
 public:
  InsnCondition(loop::LoopBlock *lb, StageScheduler *ssch);
  ~InsnCondition();

  bool Build(OptimizerLog *log);
  vector<IRegister *> GetConditions();
  bool IsCondReg(IRegister *reg);
  // In macro stage.
  int GetConditionStateIndex(IRegister *reg);
  int GetConditionLastUseStateIndex(IRegister *cond_reg);
  map<int, IRegister *> &GetConditionRegStages(IRegister *cond_reg);
  IRegister *GetFirstConditionRegStage(IRegister *cond_reg);
  IRegister *GetInsnCondition(int nthst);

 private:
  void CollectBranches();
  void PropagateCondValue(IState *branch_st);
  bool InLoop(IState *st);
  bool IsEntry(IState *st);
  void CollectReachable(IState *init_st, set<IState *> *sts);
  void CollectSideEffectInsns();
  void BuildConditionRegInfo();
  ConditionRegInfo *GetCondRegInfo(IRegister *cond_reg);
  void SetMacroStageIndex();
  void Dump(OptimizerLog *log);

  ITable *tab_;
  loop::LoopBlock *lb_;
  StageScheduler *ssch_;
  set<IState *> states_;
  vector<IState *> branches_;
  map<IState *, InsnConditionValueInfo *> cond_value_info_;
  map<IRegister *, ConditionRegInfo *> cond_reg_info_;
};

}  // namespace pipeline
}  // namespace opt
}  // namespace iroha

#endif  // _opt_pipeline_insn_condition_h_
