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
    vector<IInsn *> body_insns;
    for (IInsn *insn : st->insns_) {
      if (IsBodyInsn(insn)) {
        body_insns.push_back(insn);
      }
    }
    if (body_insns.size() == 0) {
      continue;
    }
    MacroStage ms;
    ms.insns_ = body_insns;
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

bool StageScheduler::IsBodyInsn(IInsn *insn) {
  IInsn *compare_insn = lb_->GetCompareInsn();
  if (insn == compare_insn) {
    return false;
  }
  IResource *res = insn->GetResource();
  if (resource::IsTransition(*(res->GetClass()))) {
    return false;
  }
  return true;
}

}  // namespace pipeline
}  // namespace opt
}  // namespace iroha
