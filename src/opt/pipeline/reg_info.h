// -*- C++ -*-
#ifndef _opt_pipeline_reg_info_h_
#define _opt_pipeline_reg_info_h_

#include "opt/common.h"

namespace iroha {
namespace opt {
namespace pipeline {

// Write -> Last read info.
class WRDep {
 public:
  // state in the loop.
  int wst_index_;
  int rst_index_;
  // keyed by state index in the loop.
  // write in index-th state and read in index+1th state.
  map<int, IRegister *> loop_state_regs_;
};

class RegInfo {
 public:
  RegInfo(loop::LoopBlock *lb);
  ~RegInfo();

  bool BuildWRDep(OptimizerLog *opt_log);

  loop::LoopBlock *lb_;
  map<IRegister *, WRDep *> wr_deps_;
};

}  // namespace pipeline
}  // namespace opt
}  // namespace iroha

#endif  // _opt_pipeline_reg_info_h_
