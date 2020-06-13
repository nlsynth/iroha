// -*- C++ -*-
#ifndef _opt_sched_transition_fixup_h_
#define _opt_sched_transition_fixup_h_

#include "opt/sched/common.h"

namespace iroha {
namespace opt {
namespace sched {

// This takes care of transition instructions as BBScheduler cares other insns.
class TransitionFixup {
public:
  TransitionFixup(DataPathSet *dps);

  void Perform();

private:
  void ProcessBB(BBDataPath *bbp);

  DataPathSet *dps_;
};

}  // namespace sched
}  // namespace opt
}  // namespace iroha

#endif  // _opt_sched_transition_fixup_h_
