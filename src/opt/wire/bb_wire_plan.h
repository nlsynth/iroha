// -*- C++ -*-
#ifndef _opt_wire_bb_wire_plan_h_
#define _opt_wire_bb_wire_plan_h_

#include "opt/wire/common.h"

namespace iroha {
namespace opt {
namespace wire {

class BBWirePlan {
public:
  BBWirePlan(BBDataPath *dp);
  ~BBWirePlan();

  void Save();
  void Restore();

private:
  BBDataPath *dp_;
};

}  // namespace wire
}  // namespace opt
}  // namespace iroha

#endif  // _opt_wire_bb_wire_plan_h_
