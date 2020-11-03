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
  if (!BuildStageConstraints()) {
    return false;
  }
  interval_ = CalculateInterval();
  ScheduleLocalStages();
  ScheduleMacroStages();
  return ScheduleInsns();
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
  IInsn *increment_insn = lb_->GetIncrementInsn();
  if (insn == compare_insn || insn == increment_insn) {
    return false;
  }
  IResource *res = insn->GetResource();
  if (resource::IsTransition(*(res->GetClass()))) {
    return false;
  }
  return true;
}

bool StageScheduler::BuildStageConstraints() {
  auto &sts = lb_->GetStates();
  for (IState *st : sts) {
    // pushes constraint value back of stage_constraints_ on success.
    GetStageConstraint(st);
  }
  // checks if all constraints are set.
  if (sts.size() != stage_constraints_.size()) {
    return false;
  }
  return true;
}

void StageScheduler::GetStageConstraint(IState *st) {
  int c = -1;
  for (IInsn *insn : st->insns_) {
    IResource *res = insn->GetResource();
    if (resource::IsArray(*(res->GetClass()))) {
      if (insn->inputs_.size() == 1 && insn->outputs_.size() == 0) {
        // Address phase of read.
        if (c > -1 && c != 0) {
          // already has a different constraint.
          return;
        }
        c = 0;
      }
      if (insn->inputs_.size() == 0 && insn->outputs_.size() == 1) {
        // Data phase of read.
        if (c > -1 && c != 1) {
          // already has a different constraint.
          return;
        }
        c = 1;
      }
    }
  }
  stage_constraints_.push_back(c);
}

bool StageScheduler::ScheduleInsns() {
  auto &sts = lb_->GetStates();
  for (int i = 0; i < sts.size(); ++i) {
    IState *st = sts[i];
    int c = stage_constraints_[i];
    ScheduleStateInsns(st, c);
  }
  if (macro_stages_.size() == 0) {
    return false;
  }
  return true;
}

void StageScheduler::ScheduleStateInsns(IState *st, int local_index) {
  vector<IInsn *> body_insns;
  for (IInsn *insn : st->insns_) {
    if (IsBodyInsn(insn)) {
      body_insns.push_back(insn);
    }
  }
  if (body_insns.size() == 0) {
    return;
  }
  MacroStage ms;
  StageInsns si;
  si.insns_ = body_insns;
  ms.stages_.push_back(si);
  macro_stages_.push_back(ms);
}

int StageScheduler::CalculateInterval() {
  int max = 0;
  for (int c : stage_constraints_) {
    if (c > max) {
      max = c;
    }
  }
  return max + 1;
}

void StageScheduler::ScheduleLocalStages() {
  int next = 0;
  for (int c : stage_constraints_) {
    if (c >= 0 || c != next) {
      // fix up to the constraint.
      next = c;
    }
    local_stage_indexes_.push_back(next);
    ++next;
    if (next == interval_) {
      next = 0;
    }
  }
}

void StageScheduler::ScheduleMacroStages() {
  int prev = -1;
  int st = 0;
  for (int c : local_stage_indexes_) {
    if (c <= prev) {
      // proceed if current stage can't be scheduled at the same stage as the
      // previous.
      st++;
    }
    macro_stage_indexes_.push_back(st);
    prev = c;
  }
}

}  // namespace pipeline
}  // namespace opt
}  // namespace iroha
