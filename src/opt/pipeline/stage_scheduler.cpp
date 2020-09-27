#include "opt/pipeline/stage_scheduler.h"

#include "opt/loop/loop_block.h"

namespace iroha {
namespace opt {
namespace pipeline {

StageScheduler::StageScheduler(loop::LoopBlock *lb) : lb_(lb), interval_(1) {}

loop::LoopBlock *StageScheduler::GetLoop() { return lb_; }

bool StageScheduler::Build() { return true; }

vector<IState *> StageScheduler::GetStates() { return lb_->GetStates(); }

int StageScheduler::GetInterval() { return interval_; }

int StageScheduler::GetStageCount() {
  int ns = GetStates().size();
  return (ns + (ns - 1)) * interval_;
}

}  // namespace pipeline
}  // namespace opt
}  // namespace iroha
