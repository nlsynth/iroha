// -*- C++ -*-
#ifndef _opt_pipeline_stage_scheduler_h_
#define _opt_pipeline_stage_scheduler_h_

#include "opt/common.h"

namespace iroha {
namespace opt {
namespace pipeline {

class StageScheduler {
 public:
  StageScheduler(loop::LoopBlock *lb);

  bool Build();
  loop::LoopBlock *GetLoop();
  vector<IState *> GetStates();
  int GetInterval();
  int GetStageCount();

 private:
  loop::LoopBlock *lb_;
  int interval_;
};

}  // namespace pipeline
}  // namespace opt
}  // namespace iroha

#endif  // _opt_pipeline_stage_scheduler_h_
