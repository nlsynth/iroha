#include "opt/wire/scheduler.h"

#include "opt/wire/data_path.h"

namespace iroha {
namespace opt {
namespace wire {

Scheduler::Scheduler(DataPathSet *data_path_set)
  : data_path_set_(data_path_set) {
}

void Scheduler::Schedule() {
  auto &paths = data_path_set_->GetPaths();
  for (auto &p : paths) {
    ScheduleForDataPath(p.second);
  }
}

void Scheduler::ScheduleForDataPath(DataPath *dp) {
}

}  // namespace wire
}  // namespace opt
}  // namespace iroha
