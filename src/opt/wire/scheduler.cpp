#include "opt/wire/scheduler.h"

#include "design/resource_attr.h"
#include "iroha/i_design.h"
#include "opt/bb_set.h"
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

Scheduler::Scheduler(BBDataPath *data_path, DelayInfo *delay_info)
  : data_path_(data_path), delay_info_(delay_info) {
}

bool Scheduler::IsSchedulable() {
  // TODO: We might split a basic block before/after special insns, because
  // this is too awkward.
  BB *bb = data_path_->GetBB();
  for (IState *st : bb->states_) {
    for (IInsn *insn : st->insns_) {
      if (ResourceAttr::IsExtAccessInsn(insn) ||
	  ResourceAttr::IsExtWaitInsn(insn)) {
	return false;
      }
      // This is really awkward...
      if (insn->depending_insns_.size() > 0) {
	return false;
      }
    }
  }
  return true;
}

void Scheduler::Schedule() {
  if (!IsSchedulable()) {
    return;
  }
  ClearSchedule();
  // Sort by accumlated latency from leafs.
  auto &nodes = data_path_->GetNodes();
  for (auto &p : nodes) {
    PathNode *n = p.second;
    sorted_nodes_[n->accumlated_delay_].push_back(n);
  }
  // Iterate from leafs.
  bool has_unscheduled;
  do {
    has_unscheduled = false;
    for (auto &lt : sorted_nodes_) {
      auto &ev = lt.second;
      for (auto *n : ev) {
	if (!ScheduleNode(n)) {
	  has_unscheduled = true;
	}
      }
    }
  } while (has_unscheduled);
}

bool Scheduler::ScheduleNode(PathNode *n) {
  if (n->final_st_index_ > -1) {
    // already scheduled.
    return true;
  }
  int min_st_index = 0;
  // Determines the location.
  for (auto &s : n->source_edges_) {
    PathNode *source_node = s.second->source_node_;
    if (source_node->final_st_index_ < 0) {
      // not yet scheduled. fail and try later again.
      return false;
    }
    if (min_st_index < source_node->final_st_index_) {
      min_st_index = source_node->final_st_index_;
    }
    if (min_st_index == source_node->final_st_index_) {
      int tmp_local_delay =
	source_node->state_local_delay_ + n->node_delay_;
      if (tmp_local_delay > delay_info_->GetMaxDelay()) {
	// Go to next state.
	++min_st_index;
      }
    }
  }
  // Calculates local delay.
  int source_local_delay = 0;
  for (auto &s : n->source_edges_) {
    PathNode *source_node = s.second->source_node_;
    if (source_node->final_st_index_ < min_st_index) {
      continue;
    }
    if (source_local_delay < source_node->state_local_delay_) {
      source_local_delay = source_node->state_local_delay_;
    }
  }
  if (n->GetInsn()->GetResource()->GetClass()->IsExclusive()) {
    ScheduleExclusive(n, min_st_index, source_local_delay);
  } else {
    ScheduleNonExclusive(n, min_st_index, source_local_delay);
  }

  return true;
}

void Scheduler::ClearSchedule() {
  auto &nodes = data_path_->GetNodes();
  for (auto &p : nodes) {
    PathNode *n = p.second;
    n->final_st_index_ = -1;
    n->state_local_delay_ = 0;
  }
}

void Scheduler::ScheduleExclusive(PathNode *n, int min_index,
				  int source_local_delay) {
  int loc = min_index;
  while (true) {
    auto key = std::make_tuple(n->GetInsn()->GetResource(), loc);
    auto it = resource_slots_.find(key);
    if (it == resource_slots_.end()) {
      resource_slots_.insert(key);
      break;
    }
    ++loc;
  }
  n->state_local_delay_ = n->node_delay_;
  if (loc == min_index) {
    n->state_local_delay_ += source_local_delay;
  }
  n->final_st_index_ = loc;
}

void Scheduler::ScheduleNonExclusive(PathNode *n, int min_index,
				     int source_local_delay) {
  n->final_st_index_ = min_index;
  n->state_local_delay_ = source_local_delay + n->node_delay_;
}

}  // namespace wire
}  // namespace opt
}  // namespace iroha
