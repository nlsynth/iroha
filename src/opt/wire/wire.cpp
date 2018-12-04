#include "opt/wire/wire.h"

#include "design/resource_attr.h"
#include "design/validator.h"
#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"
#include "opt/bb_set.h"
#include "opt/debug_annotation.h"
#include "opt/delay_info.h"
#include "opt/wire/data_path.h"
#include "opt/wire/explorer.h"
#include "opt/wire/relocator.h"
#include "opt/wire/resource_conflict_tracker.h"
#include "opt/wire/resource_share.h"
#include "opt/wire/scheduler.h"
#include "opt/wire/wire_plan.h"

namespace iroha {
namespace opt {
namespace wire {

Wire::Wire(ITable *table, DelayInfo *delay_info, DebugAnnotation *annotation)
  : table_(table), delay_info_(delay_info), annotation_(annotation) {
  resource_share_.reset(new ResourceShare(table));
  data_path_set_.reset(new DataPathSet());
}

Wire::~Wire() {
}

bool Wire::Perform() {
  bset_.reset(BBSet::Create(table_, annotation_));
  if (annotation_->IsEnabled()) {
    annotation_->DumpIntermediateTable(table_);
  }

  resource_share_->Scan(bset_.get());
  resource_share_->Allocate();
  resource_share_->ReBind();

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
  WirePlanSet wps(data_path_set_.get());
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

}  // namespace wire
}  // namespace opt
}  // namespace iroha
