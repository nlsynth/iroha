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
  // keyed by macro stage index in the pipeline.
  // write in index-th state and read in index+1th stage.
  map<int, IRegister *> macro_stage_regs_;
};

class RegInfo {
 public:
  RegInfo(loop::LoopBlock *lb, StageScheduler *ssch);
  ~RegInfo();

  bool BuildWRDep(OptimizerLog *opt_log);

  map<IRegister *, WRDep *> &GetWRDeps();
  WRDep *GetWRDep(IRegister *reg);

 private:
  bool BuildLoopState(OptimizerLog *opt_log);
  void SetMacroStageIndex();
  void Dump(OptimizerLog *opt_log);

  loop::LoopBlock *lb_;
  StageScheduler *ssch_;
  map<IRegister *, WRDep *> wr_deps_;
};

}  // namespace pipeline
}  // namespace opt
}  // namespace iroha

#endif  // _opt_pipeline_reg_info_h_
