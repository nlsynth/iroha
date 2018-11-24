// -*- C++ -*-
#ifndef _opt_wire_scheduler_h_
#define _opt_wire_scheduler_h_

#include "opt/wire/common.h"

#include <tuple>

namespace iroha {
namespace opt {
namespace wire {

// Schedules a DataPathSet (=ITable).
class SchedulerCore {
public:
  SchedulerCore(DataPathSet *data_path_set, DelayInfo *delay_info);

  void Schedule();

private:
  DataPathSet *data_path_set_;
  DelayInfo *delay_info_;
};

class BBResourceTracker {
public:
  ~BBResourceTracker();
  bool CanUseResource(PathNode *node, int st_index);
  void AllocateResource(PathNode *node, int st_index);

private:
  set<std::tuple<IResource *, int> > resource_slots_;
};

// Schedules a BB.
class BBScheduler {
public:
  BBScheduler(BBDataPath *data_path, DelayInfo *delay_info);
  ~BBScheduler();

  void Schedule();

private:
  bool ScheduleNode(PathNode *n);
  void ClearSchedule();
  void ScheduleExclusive(PathNode *n, int min_st_index);
  void ScheduleNonExclusive(PathNode *n, int st_index);
  bool IsSchedulable();
  int GetMinStIndex(PathNode *n);
  int GetLocalDelayBeforeNode(PathNode *n, int st_index);

  BBDataPath *data_path_;
  DelayInfo *delay_info_;
  // latency to nodes.
  map<int, vector<PathNode *> > sorted_nodes_;
  std::unique_ptr<BBResourceTracker> resource_tracker_;
};

}  // namespace wire
}  // namespace opt
}  // namespace iroha

#endif  // _opt_wire_scheduler_h_
