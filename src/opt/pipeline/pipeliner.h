// -*- C++ -*-
#ifndef _opt_pipeline_pipeliner_h_
#define _opt_pipeline_pipeliner_h_

#include "opt/common.h"

namespace iroha {
namespace opt {
namespace pipeline {

class StageScheduler;

class Pipeliner {
 public:
  Pipeliner(ITable *tab, StageScheduler *ssch);

  bool Pipeline();

 private:
  void PlaceState(int pidx, int idx);
  void ConnectPipelineState();
  void ConnectPipeline();
  void SetupCounter();
  void SetupCounterIncrement();
  void SetupExit();
  void UpdateCounterRead();
  string RegName(const string &base, int index);

  ITable *tab_;
  StageScheduler *ssch_;
  loop::LoopBlock *lb_;
  int interval_;
  OptimizerLog *opt_log_;
  // prologue -> pipeline[0] -> pipeline[1] ...
  IState *prologue_st_;
  vector<IState *> pipeline_stages_;
  vector<IRegister *> counters_;
  vector<IRegister *> counter_wires_;

  map<IInsn *, int> insn_to_stage_;
};

}  // namespace pipeline
}  // namespace opt
}  // namespace iroha

#endif  // _opt_pipeline_pipeliner_h_
