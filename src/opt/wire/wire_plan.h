// -*- C++ -*-
#ifndef _opt_wire_wire_plan_h_
#define _opt_wire_wire_plan_h_

#include "opt/wire/common.h"

namespace iroha {
namespace opt {
namespace wire {

// A snapshot of scheduling and allocation result.
class WirePlan {
public:
  WirePlan(DataPathSet *dps);

  void Save();
  void Restore();

private:
  DataPathSet *dps_;
};

class WirePlanSet {
public:
  WirePlanSet(DataPathSet *dps);
  ~WirePlanSet();

  void Save();
  void ApplyBest();

private:
  DataPathSet *dps_;
  vector<WirePlan *> plans_;
};

}  // namespace wire
}  // namespace opt
}  // namespace iroha

#endif  // _opt_wire_wire_plan_h_
