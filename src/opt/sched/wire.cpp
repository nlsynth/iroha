#include "opt/sched/wire.h"

#include "design/resource_attr.h"
#include "design/validator.h"
#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"
#include "opt/bb_set.h"
#include "opt/debug_annotation.h"
#include "opt/delay_info.h"
#include "opt/sched/data_path.h"
#include "opt/sched/explorer.h"
#include "opt/sched/plan_evaluator.h"
#include "opt/sched/relocator.h"
#include "opt/sched/resource_conflict_tracker.h"
#include "opt/sched/scheduler.h"
#include "opt/sched/wire_plan.h"

namespace iroha {
namespace opt {
namespace sched {

Wire::Wire(ITable *table, DelayInfo *delay_info, DebugAnnotation *annotation)
  : table_(table), delay_info_(delay_info), annotation_(annotation) {
  data_path_set_.reset(new DataPathSet());
}

Wire::~Wire() {
}

bool Wire::Perform() {
  bset_.reset(BBSet::Create(table_, annotation_));
  if (annotation_->IsEnabled()) {
    annotation_->DumpIntermediateTable(table_);
  }

  // Assign ids to newly allocated insns.
  Validator::ValidateTable(table_);

  data_path_set_->Build(bset_.get());
  data_path_set_->SetDelay(delay_info_);
  if (annotation_->IsEnabled()) {
    annotation_->StartSubSection("data_path", false);
    data_path_set_->Dump(annotation_);
    annotation_->ClearSubSection();
  }

  IterateScheduling();

  Relocator rel(data_path_set_.get());
  rel.Relocate();

  // Assign ids to newly allocated insns.
  Validator::ValidateTable(table_);

  if (annotation_->IsEnabled()) {
    annotation_->DumpIntermediateTable(table_);
  }

  return true;
}

void Wire::IterateScheduling() {
  PlanEvaluator ev(data_path_set_.get());
  WirePlanSet wps(data_path_set_.get(), &ev);
  Explorer explorer(&wps);
  explorer.SetInitialAllocation();

  do {
    SchedulerCore sch(data_path_set_.get(), delay_info_);
    sch.Schedule();
    auto *conflict_tracker = sch.AcquireConflictTracker();
    wps.Save(conflict_tracker);

    if (annotation_->IsEnabled()) {
      annotation_->StartSubSection("sched", false);
      conflict_tracker->Dump(annotation_);
      annotation_->ClearSubSection();
    }
  } while (explorer.MaySetNextAllocationPlan());

  wps.ApplyBest();
}

}  // namespace sched
}  // namespace opt
}  // namespace iroha
