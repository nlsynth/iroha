// -*- C++ -*-
#ifndef _opt_wire_explorer_h_
#define _opt_wire_explorer_h_

#include "opt/wire/common.h"

namespace iroha {
namespace opt {
namespace wire {

class Explorer {
public:
  Explorer(WirePlanSet *wps);

  void SetInitialAllocation();
  bool MaySetNextAllocationPlan();

private:
  WirePlanSet *wps_;
};

}  // namespace wire
}  // namespace opt
}  // namespace iroha

#endif  // _opt_wire_explorer_h_
