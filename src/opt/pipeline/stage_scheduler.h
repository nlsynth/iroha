// -*- C++ -*-
#ifndef _opt_pipeline_stage_scheduler_h_
#define _opt_pipeline_stage_scheduler_h_

#include "opt/common.h"

namespace iroha {
namespace opt {
namespace pipeline {

// Certain stage with 0, 1, ... interval -1.
class MacroStage {
 public:
  vector<IInsn *> insns_;
};

class StageScheduler {
 public:
  StageScheduler(loop::LoopBlock *lb);

  bool Build();
  loop::LoopBlock *GetLoop();
  int GetInterval();
  int GetPipelineStageLength();
  int GetMacroStageCount();
  MacroStage &GetMacroStage(int s);

 private:
  bool IsBodyInsn(IInsn *insn);

  loop::LoopBlock *lb_;
  int interval_;
  vector<MacroStage> macro_stages_;
};

}  // namespace pipeline
}  // namespace opt
}  // namespace iroha

#endif  // _opt_pipeline_stage_scheduler_h_
