// -*- C++ -*-
#ifndef _opt_pipeline_reg_info_h_
#define _opt_pipeline_reg_info_h_

#include "opt/common.h"

namespace iroha {
namespace opt {
namespace pipeline {

class StageScheduler;

// Write -> Last read info.
class WRDep {
 public:
  // state in the loop.
  int write_lst_index_;
  int read_lst_index_;
  // state in macro stage.
  int write_mst_index_;
  int read_mst_index_;
  // keyed by state index in the loop.
  // write in index-th state and read in index+1th state.
  map<int, IRegister *> loop_state_regs_;
};

class RegInfo {
 public:
  RegInfo(loop::LoopBlock *lb);
  ~RegInfo();

  bool BuildWRDep(StageScheduler *ssch, OptimizerLog *opt_log);

  map<IRegister *, WRDep *> &GetWRDeps();
  WRDep *GetWRDep(IRegister *reg);

 private:
  bool BuildLoopState(OptimizerLog *opt_log);
  void SetMacroStageIndex(StageScheduler *ssch);
  void Dump(OptimizerLog *opt_log);

  loop::LoopBlock *lb_;
  map<IRegister *, WRDep *> wr_deps_;
};

}  // namespace pipeline
}  // namespace opt
}  // namespace iroha

#endif  // _opt_pipeline_reg_info_h_
