// -*- C++ -*-
#ifndef _opt_wire_scheduler_h_
#define _opt_wire_scheduler_h_

#include "opt/wire/common.h"

#include <tuple>

namespace iroha {
namespace opt {
namespace wire {

class SchedulerCore {
public:
  SchedulerCore(DataPathSet *data_path_set, DelayInfo *delay_info);

  void Schedule();

private:
  DataPathSet *data_path_set_;
  DelayInfo *delay_info_;
};

class BBScheduler {
public:
  BBScheduler(BBDataPath *data_path, DelayInfo *delay_info);

  void Schedule();

private:
  bool ScheduleNode(PathNode *n);
  void ClearSchedule();
  void ScheduleExclusive(PathNode *n, int min_index, int source_local_delay);
  void ScheduleNonExclusive(PathNode *n, int min_index, int source_local_delay);
  bool IsSchedulable();

  BBDataPath *data_path_;
  DelayInfo *delay_info_;
  // latency to nodes.
  map<int, vector<PathNode *> > sorted_nodes_;
  set<std::tuple<IResource *, int> > resource_slots_;
};

}  // namespace wire
}  // namespace opt
}  // namespace iroha

#endif  // _opt_wire_scheduler_h_
