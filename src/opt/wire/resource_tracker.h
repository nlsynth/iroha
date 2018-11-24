// -*- C++ -*-
#ifndef _opt_wire_resource_tracker_h_
#define _opt_wire_resource_tracker_h_

#include "opt/wire/common.h"

namespace iroha {
namespace opt {
namespace wire {

class BBResourceTracker {
public:
  ~BBResourceTracker();
  bool CanUseResource(PathNode *node, int st_index);
  void AllocateResource(PathNode *node, int st_index);

private:
  set<std::tuple<IResource *, int> > resource_slots_;
};

}  // namespace wire
}  // namespace opt
}  // namespace iroha

#endif  // _opt_wire_resource_tracker_h_
