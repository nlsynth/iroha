#include "opt/wire/scheduler.h"

#include "design/resource_attr.h"
#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "opt/bb_set.h"
#include "opt/delay_info.h"
#include "opt/wire/data_path.h"
#include "opt/wire/path_node.h"
#include "opt/wire/resource_conflict_tracker.h"
#include "opt/wire/resource_tracker.h"

namespace iroha {
namespace opt {
namespace wire {

SchedulerCore::SchedulerCore(DataPathSet *data_path_set, DelayInfo *delay_info)
  : data_path_set_(data_path_set), delay_info_(delay_info) {
  conflict_tracker_.reset(new ResourceConflictTracker);
}

SchedulerCore::~SchedulerCore() {
}

void SchedulerCore::Schedule() {
  auto &paths = data_path_set_->GetPaths();
  for (auto &p : paths) {
    BBScheduler sch(p.second, delay_info_, conflict_tracker_.get());
    sch.Schedule();
  }
}

ResourceConflictTracker *SchedulerCore::AcquireConflictTracker() {
  return conflict_tracker_.release();
}

BBScheduler::BBScheduler(BBDataPath *data_path, DelayInfo *delay_info,
			 ResourceConflictTracker *conflict_tracker)
  : data_path_(data_path), delay_info_(delay_info),
    conflict_tracker_(conflict_tracker),
    resource_tracker_(new BBResourceTracker()) {
}

BBScheduler::~BBScheduler() {
}

bool BBScheduler::IsSchedulable() {
  // TODO: We might split a basic block before/after special insns, because
  // this is too awkward.
  BB *bb = data_path_->GetBB();
  for (IState *st : bb->states_) {
    for (IInsn *insn : st->insns_) {
      if (ResourceAttr::IsMultiCycleInsn(insn)) {
	return false;
      }
      if (ResourceAttr::IsExtAccessInsn(insn) ||
	  ResourceAttr::IsExtWaitInsn(insn)) {
	return false;
      }
      if (ResourceAttr::IsOrderedResource(insn->GetResource())) {
	return false;
      }
      if (resource::IsExtCombinational(*(insn->GetResource()->GetClass()))) {
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

void BBScheduler::Schedule() {
  if (!IsSchedulable()) {
    return;
  }
  ClearSchedule();
  // Sort by accumlated latency from leafs.
  int num_states = data_path_->GetBB()->states_.size();
  auto &nodes = data_path_->GetNodes();
  for (auto &p : nodes) {
    PathNode *n = p.second;
    if (n->IsTransition()) {
      // Schedule transition insns in the original order.
      // (optional: schedule them earlier than other insns by using
      // negative sort key)
      int idx = n->GetInitialStIndex() - num_states;
      sorted_nodes_[idx].push_back(n);
    } else {
      sorted_nodes_[n->GetAccumlatedDelayFromLeaf()].push_back(n);
    }
  }
  // Iterate from leafs.
  bool has_unscheduled;
  do {
    has_unscheduled = false;
    for (auto &lt : sorted_nodes_) {
      auto &ev = lt.second;
      for (auto *n : ev) {
	if (ScheduleNode(n)) {
	  UpdateLastWrite(n);
	} else {
	  has_unscheduled = true;
	}
      }
    }
  } while (has_unscheduled);
}

bool BBScheduler::ScheduleNode(PathNode *n) {
  if (n->GetFinalStIndex() > -1) {
    // already scheduled.
    return true;
  }
  // Minimum state index by src register position.
  int min_st_index = GetMinStIndex(n);
  if (min_st_index < 0) {
    return false;
  }
  // Ordering of memory access.
  if (!CheckPrecedingNodes(n)) {
    return false;
  }
  // Calculates local delay.
  if (n->GetInsn()->GetResource()->GetClass()->IsExclusive()) {
    ScheduleExclusive(n, min_st_index);
  } else {
    ScheduleNonExclusive(n, min_st_index);
  }

  return true;
}

int BBScheduler::GetMinStIndex(PathNode *n) {
  int min_st_index = 0;
  // Determines the location.
  for (auto &s : n->source_edges_) {
    PathNode *source_node = s.second->GetSourceNode();
    if (source_node->GetFinalStIndex() < 0) {
      // Not yet scheduled. Fail and try later again.
      return -1;
    }
    if (min_st_index < source_node->GetFinalStIndex()) {
      min_st_index = source_node->GetFinalStIndex();
    }
    if (min_st_index == source_node->GetFinalStIndex()) {
      int tmp_local_delay =
	source_node->state_local_delay_ + n->GetNodeDelay();
      if (tmp_local_delay > delay_info_->GetMaxDelay()) {
	// Local margin is not sufficient. Go to the next state.
	++min_st_index;
      }
    }
  }
  int s = GetMinStByLastWrite(n);
  if (s >= 0) {
    if (s > min_st_index) {
      min_st_index = s;
    }
  }
  int t = GetMinStByPrecedingNode(n);
  if (t >= 0) {
    if (t > min_st_index) {
      min_st_index = t;
    }
  }
  return min_st_index;
}

int BBScheduler::GetLocalDelayBeforeNode(PathNode *n, int st_index) {
  int source_local_delay = 0;
  for (auto &s : n->source_edges_) {
    PathNode *source_node = s.second->GetSourceNode();
    if (source_node->GetFinalStIndex() < st_index) {
      continue;
    }
    if (source_local_delay < source_node->state_local_delay_) {
      source_local_delay = source_node->state_local_delay_;
    }
  }
  return source_local_delay;
}

void BBScheduler::ClearSchedule() {
  auto &nodes = data_path_->GetNodes();
  for (auto &p : nodes) {
    PathNode *n = p.second;
    n->SetFinalStIndex(-1);
    n->state_local_delay_ = 0;
  }
}

void BBScheduler::ScheduleExclusive(PathNode *n, int min_st_index) {
  int loc = min_st_index;
  // Tries to find a state index where this node can use the resource.
  while (true) {
    if (resource_tracker_->CanUseResource(n, loc)) {
      resource_tracker_->AllocateResource(n, loc);
      break;
    }
    ++loc;
  }
  n->state_local_delay_ = n->GetNodeDelay();
  bool has_conflict = true;
  if (loc == min_st_index) {
    has_conflict = false;
    n->state_local_delay_ += GetLocalDelayBeforeNode(n, min_st_index);
  }
  conflict_tracker_->AddUsage(n, has_conflict);
  n->SetFinalStIndex(loc);
}

void BBScheduler::ScheduleNonExclusive(PathNode *n, int st_index) {
  // Just place the insn at st_index.
  n->SetFinalStIndex(st_index);
  int source_local_delay = GetLocalDelayBeforeNode(n, st_index);
  n->state_local_delay_ = source_local_delay + n->GetNodeDelay();
}

void BBScheduler::UpdateLastWrite(PathNode *n) {
  int st_index = n->GetFinalStIndex();
  IInsn *insn = n->GetInsn();
  for (IRegister *reg : insn->outputs_) {
    int idx = last_write_index_[reg];
    if (idx < st_index) {
      last_write_index_[reg] = st_index;
    }
  }
}

int BBScheduler::GetMinStByLastWrite(PathNode *n) {
  int min = -1;
  IInsn *insn = n->GetInsn();
  for (IRegister *reg : insn->outputs_) {
    auto it = last_write_index_.find(reg);
    if (it == last_write_index_.end()) {
      continue;
    }
    int st = it->second;
    if (st > min) {
      min = st;
    }
  }
  if (min > -1) {
    return min + 1;
  }
  return -1;
}

bool BBScheduler::CheckPrecedingNodes(PathNode *n) {
  IResource *res = n->GetInsn()->GetResource();
  if (!ResourceAttr::IsOrderedResource(res)) {
    return true;
  }
  int idx = GetMinStByPrecedingNode(n);
  if (idx < 0) {
    // Preceding node is not scheduled.
    return false;
  }
  return true;
}

int BBScheduler::GetMinStByPrecedingNode(PathNode *n) {
  IResource *res = n->GetInsn()->GetResource();
  if (!ResourceAttr::IsOrderedResource(res)) {
    return -1;
  }
  auto &m = data_path_->GetResourceNodeMap(res);
  auto it = m.find(n->GetInitialStIndex());
  if (it == m.begin()) {
    return 0;
  }
  it--;
  PathNode *prev = it->second;
  int st_index = prev->GetFinalStIndex();
  if (st_index > -1) {
    return st_index + 1;
  }
  return -1;
}

}  // namespace wire
}  // namespace opt
}  // namespace iroha
