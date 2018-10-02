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
  int min_st_index = 0;
  // Determines the location.
  for (auto &s : e->sources_) {
    PathEdge *source_edge = s.second;
    if (source_edge->final_st_index_ < 0) {
      // not yet scheduled. fail and try later again.
      return false;
    }
    if (min_st_index < source_edge->final_st_index_) {
      min_st_index = source_edge->final_st_index_;
      int tmp_local_delay =
	source_edge->state_local_delay_ + e->edge_delay_;
      if (tmp_local_delay > delay_info_->GetMaxDelay()) {
	// Go to next state.
	++min_st_index;
      }
    }
  }
  // Calculates local delay.
  int source_local_delay = 0;
  for (auto &s : e->sources_) {
    PathEdge *source_edge = s.second;
    if (source_edge->final_st_index_ < min_st_index) {
      continue;
    }
    if (source_local_delay < source_edge->state_local_delay_) {
      source_local_delay = source_edge->state_local_delay_;
    }
  }
  if (e->insn_->GetResource()->GetClass()->IsExclusive()) {
    ScheduleExclusive(e, min_st_index, source_local_delay);
  } else {
    ScheduleNonExclusive(e, min_st_index, source_local_delay);
  }

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

void Scheduler::ScheduleExclusive(PathEdge *e, int min_index,
				  int source_local_delay) {
  int loc = min_index;
  while (true) {
    auto key = std::make_tuple(e->insn_->GetResource(), loc);
    auto it = resource_slots_.find(key);
    if (it == resource_slots_.end()) {
      resource_slots_.insert(key);
      break;
    }
    ++loc;
  }
  e->state_local_delay_ = e->edge_delay_;
  if (loc == min_index) {
    e->state_local_delay_ += source_local_delay;
  }
  e->final_st_index_ = loc;
}

void Scheduler::ScheduleNonExclusive(PathEdge *e, int min_index,
				     int source_local_delay) {
  e->final_st_index_ = min_index;
  e->state_local_delay_ = source_local_delay + e->edge_delay_;
}

}  // namespace wire
}  // namespace opt
}  // namespace iroha
