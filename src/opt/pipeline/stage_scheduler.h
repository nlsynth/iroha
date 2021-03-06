// -*- C++ -*-
#ifndef _opt_pipeline_stage_scheduler_h_
#define _opt_pipeline_stage_scheduler_h_

#include "opt/common.h"

namespace iroha {
namespace opt {
namespace pipeline {

// Certain stage with 0, 1, ... interval -1.
class StageInsns {
 public:
  int loop_state_index_;
  vector<IInsn *> insns_;
};

class MacroStage {
 public:
  vector<StageInsns> local_stages_;
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
  int GetMacroStageFromLoopStateIndex(int i);

 private:
  bool IsBodyInsn(IInsn *insn);
  bool ScheduleInsns();
  void ScheduleStateInsns(IState *st, int macro_index, int local_index,
                          int loop_state_index);
  bool BuildStageConstraints();
  void GetStageConstraint(IState *st);
  int CalculateInterval();
  void ScheduleLocalStages();
  void ScheduleMacroStages();
  void PrepareStages();

  loop::LoopBlock *lb_;
  int interval_;
  vector<MacroStage> macro_stages_;
  // [loop state index] = 0, 1,,, (-1 for unconstrainted).
  vector<int> stage_constraints_;
  // Indexed by loop state index.
  vector<int> loop_state_to_local_stage_;
  vector<int> loop_state_to_macro_stage_;
};

}  // namespace pipeline
}  // namespace opt
}  // namespace iroha

#endif  // _opt_pipeline_stage_scheduler_h_
