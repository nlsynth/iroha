#include "opt/wire/scheduler.h"

#include "iroha/i_design.h"
#include "opt/delay_info.h"
#include "opt/wire/data_path.h"

namespace iroha {
namespace opt {
namespace wire {

SchedulerCore::SchedulerCore(DataPathSet *data_path_set, DelayInfo *delay_info)
  : data_path_set_(data_path_set), delay_info_(delay_info) {
}

void SchedulerCore::Schedule() {
  auto &paths = data_path_set_->GetPaths();
  for (auto &p : paths) {
    Scheduler sch(p.second, delay_info_);
    sch.Schedule();
  }
}

Scheduler::Scheduler(DataPath *data_path, DelayInfo *delay_info)
  : data_path_(data_path), delay_info_(delay_info) {
}

void Scheduler::Schedule() {
  ClearSchedule();
  // Sorted by latency.
  auto &edges = data_path_->GetEdges();
  for (auto &p : edges) {
    PathEdge *e = p.second;
    sorted_edges_[e->accumlated_delay_].push_back(e);
  }
  // Iterate from leaf.
  bool has_unscheduled;
  do {
    has_unscheduled = false;
    for (auto &lt : sorted_edges_) {
      auto &ev = lt.second;
      for (auto *e : ev) {
	if (!ScheduleEdge(e)) {
	  has_unscheduled = true;
	}
      }
    }
  } while (has_unscheduled);
}

bool Scheduler::ScheduleEdge(PathEdge *e) {
  if (e->final_st_index_ > -1) {
    // already scheduled.
    return true;
  }
  int max_index = 0;
  for (auto &s : e->sources_) {
    PathEdge *source_edge = s.second;
    if (source_edge->final_st_index_ < 0) {
      return false;
    }
    if (max_index < source_edge->final_st_index_) {
      max_index = source_edge->final_st_index_;
    }
  }
  if (!e->insn_->GetResource()->GetClass()->IsExclusive()) {
    ScheduleNonExclusive(e, max_index);
    return true;
  }

  ScheduleExclusive(e, max_index);
  return true;
}

void Scheduler::ClearSchedule() {
  auto &edges = data_path_->GetEdges();
  for (auto &p : edges) {
    PathEdge *e = p.second;
    e->final_st_index_ = -1;
    e->state_local_delay_ = 0;
  }
}

void Scheduler::ScheduleExclusive(PathEdge *e, int min_index) {
  // TODO: Implement this.
  e->final_st_index_ = e->initial_st_index_;
}

void Scheduler::ScheduleNonExclusive(PathEdge *e, int min_index) {
  int max_delay = 0;
  for (auto &s : e->sources_) {
    PathEdge *source_edge = s.second;
    if (source_edge->final_st_index_ < min_index) {
      continue;
    }
    if (max_delay < source_edge->state_local_delay_) {
      max_delay = source_edge->state_local_delay_;
    }
  }
  max_delay += e->edge_delay_;
  if (max_delay < delay_info_->GetMaxDelay()) {
    e->final_st_index_ = min_index;
    e->state_local_delay_ = max_delay;
  } else {
    e->final_st_index_ = min_index + 1;
    e->state_local_delay_ = e->edge_delay_;
  }
}

}  // namespace wire
}  // namespace opt
}  // namespace iroha
