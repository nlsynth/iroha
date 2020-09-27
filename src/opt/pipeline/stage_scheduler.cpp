#include "opt/pipeline/stage_scheduler.h"

#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "opt/loop/loop_block.h"

namespace iroha {
namespace opt {
namespace pipeline {

StageScheduler::StageScheduler(loop::LoopBlock *lb) : lb_(lb), interval_(1) {}

loop::LoopBlock *StageScheduler::GetLoop() { return lb_; }

bool StageScheduler::Build() {
  auto &sts = lb_->GetStates();
  for (IState *st : sts) {
    bool has_non_tr = false;
    for (IInsn *insn : st->insns_) {
      IResource *res = insn->GetResource();
      if (!resource::IsTransition(*(res->GetClass()))) {
        has_non_tr = true;
      }
    }
    if (!has_non_tr) {
      continue;
    }
    MacroStage ms;
    ms.insns_ = st->insns_;
    macro_stages_.push_back(ms);
  }
  return true;
}

MacroStage &StageScheduler::GetMacroStage(int s) { return macro_stages_[s]; }

int StageScheduler::GetMacroStageCount() { return macro_stages_.size(); }

int StageScheduler::GetInterval() { return interval_; }

int StageScheduler::GetPipelineStageLength() {
  int ns = GetMacroStageCount();
  return (2 * ns - 1) * interval_;
}

}  // namespace pipeline
}  // namespace opt
}  // namespace iroha
