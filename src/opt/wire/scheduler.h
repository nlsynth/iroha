// -*- C++ -*-
#ifndef _opt_wire_scheduler_h_
#define _opt_wire_scheduler_h_

#include "opt/wire/common.h"

namespace iroha {
namespace opt {
namespace wire {

class SchedulerCore {
public:
  SchedulerCore(DataPathSet *data_path_set);

  void Schedule();

private:
  DataPathSet *data_path_set_;
};

class Scheduler {
public:
  Scheduler(DataPath *data_path);

  void Schedule();

private:
  void ScheduleEdge(PathEdge *e);

  DataPath *data_path_;
  // latency to edges.
  map<int, vector<PathEdge *> > sorted_edges_;
};

}  // namespace wire
}  // namespace opt
}  // namespace iroha

#endif  // _opt_wire_scheduler_h_
