#include "opt/sched/bb_scheduler.h"

#include "design/resource_attr.h"
#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "opt/bb_set.h"
#include "opt/delay_info.h"
#include "opt/sched/bb_data_path.h"
#include "opt/sched/data_path_set.h"
#include "opt/sched/path_node.h"
#include "opt/sched/resource_conflict_tracker.h"
#include "opt/sched/resource_tracker.h"

namespace iroha {
namespace opt {
namespace sched {

BBScheduler::BBScheduler(BBDataPath *data_path, DelayInfo *delay_info,
			 ResourceConflictTracker *conflict_tracker)
  : data_path_(data_path), delay_info_(delay_info),
    conflict_tracker_(conflict_tracker),
    resource_tracker_(new BBResourceTracker()) {
}

BBScheduler::~BBScheduler() {
}

bool BBScheduler::IsSchedulable() {
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
      auto rc = *(insn->GetResource()->GetClass());
      if (resource::IsExtCombinational(rc)) {
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
	if (!ScheduleNode(n)) {
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
  if (!CheckPrecedingNodesOfSameResource(n)) {
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
  // Checks if all preceding nodes (W->W, W->R, R->W, DEP) are scheduled.
  for (auto &s : n->source_edges_) {
    PathEdge *edge = s.second;
    PathNode *source_node = edge->GetSourceNode();
    if (source_node->GetFinalStIndex() < 0) {
      // Not yet scheduled. Fail and try later again.
      return -1;
    }
  }
  // Determines the location.
  int min_st_index = 0;
  for (auto &s : n->source_edges_) {
    PathEdge *edge = s.second;
    if (!edge->IsWtoR()) {
      continue;
    }
    PathNode *source_node = edge->GetSourceNode();
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
  int t = GetMinStByPrecedingNodeOfSameResource(n);
  if (t >= 0) {
    if (t > min_st_index) {
      min_st_index = t;
    }
  }
  int u = GetMinStByEdgeDependency(n);
  if (u >= 0) {
    if (u >= min_st_index) {
      min_st_index = u;
    }
  }
  return min_st_index;
}

int BBScheduler::GetLocalDelayBeforeNode(PathNode *n, int st_index) {
  int source_local_delay = 0;
  for (auto &s : n->source_edges_) {
    PathEdge *edge = s.second;
    if (!edge->IsWtoR()) {
      continue;
    }
    PathNode *source_node = edge->GetSourceNode();
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

int BBScheduler::GetMinStByEdgeDependency(PathNode *n) {
  int min = -1;
  for (auto &s : n->source_edges_) {
    PathEdge *edge = s.second;
    // This takes care of W->W or R->W.
    if (edge->IsWtoR()) {
      continue;
    }
    PathNode *source_node = edge->GetSourceNode();
    if (source_node->GetFinalStIndex() > min) {
      min = source_node->GetFinalStIndex();
    }
  }
  if (min >= 0) {
    return min + 1;
  }
  return -1;
}

bool BBScheduler::CheckPrecedingNodesOfSameResource(PathNode *n) {
  IResource *res = n->GetInsn()->GetResource();
  if (!ResourceAttr::IsOrderedResource(res)) {
    // Skips normal resources (other than array access or so on).
    return true;
  }
  int idx = GetMinStByPrecedingNodeOfSameResource(n);
  if (idx < 0) {
    // Preceding node is not scheduled.
    return false;
  }
  return true;
}

int BBScheduler::GetMinStByPrecedingNodeOfSameResource(PathNode *n) {
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

}  // namespace sched
}  // namespace opt
}  // namespace iroha
