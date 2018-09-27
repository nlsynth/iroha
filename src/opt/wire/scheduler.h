// -*- C++ -*-
#ifndef _opt_wire_scheduler_h_
#define _opt_wire_scheduler_h_

#include "opt/wire/common.h"

namespace iroha {
namespace opt {
namespace wire {

class Scheduler {
public:
  Scheduler(DataPathSet *dataPathSet);

  void Schedule();

private:
  DataPathSet *dataPathSet_;
};

}  // namespace wire
}  // namespace opt
}  // namespace iroha

#endif  // _opt_wire_scheduler_h_
