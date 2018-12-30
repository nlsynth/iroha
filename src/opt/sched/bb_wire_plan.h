// -*- C++ -*-
#ifndef _opt_sched_bb_wire_plan_h_
#define _opt_sched_bb_wire_plan_h_

#include "opt/sched/common.h"

namespace iroha {
namespace opt {
namespace sched {

class BBWirePlan {
public:
  BBWirePlan(BBDataPath *dp);
  ~BBWirePlan();

  void Save();
  void Restore();
  BBDataPath *GetBBDataPath();
  map<PathNode *, int> &GetStIndexes();

private:
  BBDataPath *dp_;
  map<PathNode *, int> st_indexes_;
};

}  // namespace sched
}  // namespace opt
}  // namespace iroha

#endif  // _opt_sched_bb_wire_plan_h_
