#include "opt/sched/table_scheduler.h"

#include "design/validator.h"
#include "iroha/i_design.h"
#include "iroha/resource_attr.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"
#include "opt/bb_set.h"
#include "opt/delay_info.h"
#include "opt/optimizer_log.h"
#include "opt/sched/bb_data_path.h"
#include "opt/sched/bb_scheduler.h"
#include "opt/sched/data_path_set.h"
#include "opt/sched/explorer.h"
#include "opt/sched/relocator.h"
#include "opt/sched/resource_conflict_tracker.h"
#include "opt/sched/transition_fixup.h"
#include "opt/sched/wire_plan.h"

namespace iroha {
namespace opt {
namespace sched {

// Schedules a DataPathSet (=ITable).
class SchedulerCore {
 public:
  SchedulerCore(DataPathSet *data_path_set, DelayInfo *delay_info);
  ~SchedulerCore();

  void Schedule();
  // Takes the ownership of the object.
  ResourceConflictTracker *AcquireConflictTracker();

 private:
  DataPathSet *data_path_set_;
  DelayInfo *delay_info_;
  std::unique_ptr<ResourceConflictTracker> conflict_tracker_;
};

TableScheduler::TableScheduler(ITable *table, DelayInfo *delay_info,
                               OptimizerLog *opt_log)
    : table_(table), delay_info_(delay_info), opt_log_(opt_log) {
  data_path_set_.reset(new DataPathSet());
}

TableScheduler::~TableScheduler() {}

bool TableScheduler::Perform() {
  bset_.reset(BBSet::Create(table_, true, opt_log_));
  if (opt_log_->IsEnabled()) {
    opt_log_->DumpIntermediateTable(table_);
  }

  // Assign ids to newly allocated insns.
  Validator::ValidateTable(table_);

  data_path_set_->Build(bset_.get());
  data_path_set_->SetDelay(delay_info_);
  if (opt_log_->IsEnabled()) {
    opt_log_->StartSubSection("data_path", false);
    data_path_set_->Dump(opt_log_);
    opt_log_->ClearSubSection();
  }

  IterateScheduling();

  TransitionFixup tf(data_path_set_.get());
  tf.Perform();

  Relocator rel(data_path_set_.get());
  rel.Relocate();

  // Assign ids to newly allocated insns.
  Validator::ValidateTable(table_);

  if (opt_log_->IsEnabled()) {
    opt_log_->DumpIntermediateTable(table_);
  }

  return true;
}

void TableScheduler::IterateScheduling() {
  Explorer explorer;
  explorer.SetInitialAllocation();
  WirePlanSet *wps = explorer.GetWirePlanSet();

  do {
    SchedulerCore sch(data_path_set_.get(), delay_info_);
    sch.Schedule();
    auto *conflict_tracker = sch.AcquireConflictTracker();
    wps->SaveCurrentSchedulingPlan(data_path_set_.get(), conflict_tracker);

    if (opt_log_->IsEnabled()) {
      opt_log_->StartSubSection("sched", false);
      conflict_tracker->Dump(opt_log_);
      opt_log_->ClearSubSection();
    }
  } while (explorer.MaySetNextAllocationPlan());

  wps->ApplyBest();
}

SchedulerCore::SchedulerCore(DataPathSet *data_path_set, DelayInfo *delay_info)
    : data_path_set_(data_path_set), delay_info_(delay_info) {
  conflict_tracker_.reset(new ResourceConflictTracker);
}

SchedulerCore::~SchedulerCore() {}

void SchedulerCore::Schedule() {
  auto &paths = data_path_set_->GetBBPaths();
  for (auto &p : paths) {
    BBScheduler sch(p.second, delay_info_, conflict_tracker_.get());
    sch.Schedule();
  }
}

ResourceConflictTracker *SchedulerCore::AcquireConflictTracker() {
  return conflict_tracker_.release();
}

}  // namespace sched
}  // namespace opt
}  // namespace iroha
