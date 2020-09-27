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

  loop::LoopBlock *GetLoop();
  bool Build();

 private:
  loop::LoopBlock *lb_;
};

}  // namespace pipeline
}  // namespace opt
}  // namespace iroha

#endif  // _opt_pipeline_stage_scheduler_h_
