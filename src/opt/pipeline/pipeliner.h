// -*- C++ -*-
#ifndef _opt_pipeline_pipeliner_h_
#define _opt_pipeline_pipeliner_h_

#include "opt/common.h"

namespace iroha {
namespace opt {
namespace pipeline {

class StageScheduler;
class WRDep {
 public:
  IState *wst_;
  IState *rst_;
};

class Pipeliner {
 public:
  Pipeliner(ITable *tab, StageScheduler *ssch);
  ~Pipeliner();

  bool Pipeline();

 private:
  void PlaceState(int pidx, int idx);
  IRegister *MayUpdateWireReg(IState *st, IRegister *reg);
  void UpdateRegs(IState *st, bool is_output, vector<IRegister *> &src,
                  vector<IRegister *> *dst);
  void ConnectPipelineState();
  void ConnectPipeline();
  void SetupCounter();
  void SetupCounterIncrement();
  void SetupExit();
  void UpdateCounterRead();
  string RegName(const string &base, int index);
  bool CollectWRRegs();
  void PrepareRegPipeline();
  IRegister *LookupStagedReg(IState *st, IRegister *reg);

  ITable *tab_;
  StageScheduler *ssch_;
  loop::LoopBlock *lb_;
  int interval_;
  OptimizerLog *opt_log_;
  // prologue -> pipeline[0] -> pipeline[1] ...
  IState *prologue_st_;
  vector<IState *> pipeline_stages_;
  vector<IRegister *> counters_;
  vector<IRegister *> counter_wires_;

  map<IInsn *, int> insn_to_stage_;
  // mapping from a wire in the loop to a wire in the pipeline.
  map<tuple<IState *, IRegister *>, IRegister *> wire_to_reg_;
  map<IRegister *, WRDep *> wr_deps_;
};

}  // namespace pipeline
}  // namespace opt
}  // namespace iroha

#endif  // _opt_pipeline_pipeliner_h_
