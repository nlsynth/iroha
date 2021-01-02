// -*- C++ -*-
#ifndef _opt_pipeline_pipeliner_h_
#define _opt_pipeline_pipeliner_h_

#include "opt/common.h"

namespace iroha {
namespace opt {
namespace pipeline {

class InsnCondition;
class RegInfo;
class Shape;
class StageScheduler;
class WRDep;

class Pipeliner {
 public:
  Pipeliner(ITable *tab, StageScheduler *ssch, RegInfo *reg_info,
            InsnCondition *insn_cond);
  ~Pipeliner();

  bool Pipeline();

 private:
  void PlaceState(int pipeline_macro_stage_index, int loop_macro_stage_index);
  IRegister *MayUpdateWireReg(IState *pst, IRegister *reg);
  void UpdateRegs(IState *pst, int lidx, bool is_output,
                  vector<IRegister *> &src, vector<IRegister *> *dst);
  void ConnectPipelineState();
  void ConnectPipeline();
  void SetupCounter();
  void SetupCounterIncrement();
  void SetupExit();
  void UpdateCounterRead();
  string RegName(const string &base, int index);
  void PrepareRegWriteReadPipeline();
  void PrepareInsnCondRegPipeline();
  void UpdatePipelineRegWrite();
  IRegister *LookupStagedReg(int lidx, IRegister *reg);

  ITable *tab_;
  StageScheduler *ssch_;
  RegInfo *reg_info_;
  InsnCondition *insn_cond_;
  loop::LoopBlock *lb_;
  int interval_;
  OptimizerLog *opt_log_;
  std::unique_ptr<Shape> shape_;
  // prologue -> pipeline[0] -> pipeline[1] ...
  IState *prologue_st_;
  vector<IState *> pipeline_stages_;
  vector<IRegister *> counters_;
  vector<IRegister *> counter_wires_;

  map<IInsn *, int> insn_to_stage_;
  // mapping from <pipeline state & a wire in the loop> to a wire in the
  // pipeline.
  map<tuple<IState *, IRegister *>, IRegister *> wire_to_reg_;
};

}  // namespace pipeline
}  // namespace opt
}  // namespace iroha

#endif  // _opt_pipeline_pipeliner_h_
