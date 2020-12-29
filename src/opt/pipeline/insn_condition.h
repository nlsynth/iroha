// -*- C++ -*-
#ifndef _opt_pipeline_insn_condition_h_
#define _opt_pipeline_insn_condition_h_

#include "opt/common.h"

namespace iroha {
namespace opt {
namespace pipeline {

// Finds conditional executions in a pipeline.
class InsnCondition {
 public:
  InsnCondition(loop::LoopBlock *lb);

  bool Build(OptimizerLog *log);

 private:
  void CollectBranches(OptimizerLog *log);
  void PropagateCondValue(IState *st);
  bool InLoop(IState *st);
  bool IsEntry(IState *st);
  void CollectReachable(IState *init_st, set<IState *> *sts);

  ITable *tab_;
  loop::LoopBlock *lb_;
  set<IState *> states_;
  vector<IState *> branches_;
};

}  // namespace pipeline
}  // namespace opt
}  // namespace iroha

#endif  // _opt_pipeline_insn_condition_h_
