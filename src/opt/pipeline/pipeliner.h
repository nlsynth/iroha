// -*- C++ -*-
#ifndef _opt_pipeline_pipeliner_h_
#define _opt_pipeline_pipeliner_h_

#include "opt/common.h"

namespace iroha {
namespace opt {
namespace loop {
class LoopBlock;
}  // namespace loop
namespace pipeline {

class Pipeliner {
 public:
  Pipeliner(ITable *tab, loop::LoopBlock *lb);

  bool Pipeline();

 private:
  void PlaceState(int pidx, int idx);
  void ConnectPipelineState();
  void ConnectPipeline();
  void SetupCounter();
  void SetupExit();

  ITable *tab_;
  loop::LoopBlock *lb_;
  int interval_;
  OptimizerLog *opt_log_;
  // prologue -> pipeline[0] -> pipeline[1] ...
  IState *prologue_st_;
  vector<IState *> pipeline_st_;
  vector<IRegister *> counters_;
};

}  // namespace pipeline
}  // namespace opt
}  // namespace iroha

#endif  // _opt_pipeline_pipeliner_h_