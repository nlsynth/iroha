#include "opt/sched/sched_block.h"

#include "opt/bb_set.h"
#include "iroha/stl_util.h"

namespace iroha {
namespace opt {
namespace sched {

SchedBlock::SchedBlock(BB *bb) : bb_(bb) {
}

SchedBlockSet::SchedBlockSet(BBSet *bbs) {
  for (BB *bb : bbs->bbs_) {
    fbs_.push_back(new SchedBlock(bb));
  }
}

SchedBlockSet::~SchedBlockSet() {
  STLDeleteValues(&fbs_);
}

}  // namespace sched
}  // namespace opt
}  // namespace iroha
