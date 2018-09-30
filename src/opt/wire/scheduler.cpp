#include "opt/wire/scheduler.h"

#include "opt/wire/data_path.h"

namespace iroha {
namespace opt {
namespace wire {

SchedulerCore::SchedulerCore(DataPathSet *data_path_set)
  : data_path_set_(data_path_set) {
}

void SchedulerCore::Schedule() {
  auto &paths = data_path_set_->GetPaths();
  for (auto &p : paths) {
    Scheduler sch(p.second);
    sch.Schedule();
  }
}

Scheduler::Scheduler(DataPath *data_path) : data_path_(data_path) {
}

void Scheduler::Schedule() {
  auto &edges = data_path_->GetEdges();
  for (auto &p : edges) {
    PathEdge *e = p.second;
    sorted_edges_[e->accumlated_delay_].push_back(e);
  }
  for (auto &lt : sorted_edges_) {
    auto &ev = lt.second;
    for (auto *e : ev) {
    }
  }
}

void Scheduler::ScheduleEdge(PathEdge *e) {
  e->final_st_index_ = e->initial_st_index_;
}

}  // namespace wire
}  // namespace opt
}  // namespace iroha
