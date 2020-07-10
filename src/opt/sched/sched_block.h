// -*- C++ -*-
#ifndef _opt_sched_sched_block_h_
#define _opt_sched_sched_block_h_

#include "opt/sched/common.h"

namespace iroha {
namespace opt {
namespace sched {

// This is equivalent as BB for now (WIP).
class SchedBlock {
public:
  SchedBlock(BB *bb);

  BB *bb_;
};

class SchedBlockSet {
public:
  SchedBlockSet(BBSet *bbs);
  ~SchedBlockSet();

  vector<SchedBlock*> fbs_;
};

}  // namespace sched
}  // namespace opt
}  // namespace iroha

#endif  // _opt_sched_sched_block_h_

