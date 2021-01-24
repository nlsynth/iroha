// -*- C++ -*-
#ifndef _opt_pipeline_pipeliner_h_
#define _opt_pipeline_pipeliner_h_

#include "opt/common.h"

namespace iroha {
namespace opt {
namespace pipeline {

class InsnCondition;
class RegInfo;
class ScheduledShape;
class StageScheduler;
class WRDep;

class Pipeliner {
 public:
  Pipeliner(ITable *tab, StageScheduler *ssch, RegInfo *reg_info,
            InsnCondition *insn_cond);
  ~Pipeliner();

  bool Pipeline();

 private:
  void PrepareStates();
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
  // Write -> Read deps.
  void PrepareRegWriteReadPipelineRegs();
  void PrepareRegWriteReadPipelineStages();
  void UpdateRegWriteReadPipelineAccess();
  // Condition reg access.
  void PrepareInsnCondRegPipelineStages();
  void PrepareInsnCondRegPipelineRegs();
  void AssignInitialInsnCondRegs();
  void SetInsnCondRegs();

  void PipelineRegs(int start, int end, map<int, IRegister *> &regs);
  IRegister *LookupStagedReg(int lidx, IRegister *reg);
  IRegister *LookupOriginalWire(IRegister *wire_reg);
  IRegister *Negate(IRegister *reg, IState *st);
  // returns assign insn which assigns wire to reg.
  IInsn *InjectOutputWire(IInsn *insn, int nth);

  ITable *tab_;
  StageScheduler *ssch_;
  RegInfo *reg_info_;
  InsnCondition *insn_cond_;
  loop::LoopBlock *lb_;
  int interval_;
  OptimizerLog *opt_log_;
  std::unique_ptr<ScheduledShape> scheduled_shape_;
  IResource *assign_;
  // prologue -> pipeline[0] -> pipeline[1] ...
  IState *prologue_st_;
  vector<IState *> pipeline_stages_;
  // key-ed by macro stage (0..2n-1)
  vector<IRegister *> counters_;
  vector<IRegister *> counter_wires_;

  map<IInsn *, int> insn_to_macro_stage_;
  map<IInsn *, int> insn_to_loop_state_index_;
  // mapping from <pipeline state & a wire in the loop> to a wire in the
  // pipeline.
  map<tuple<IState *, IRegister *>, IRegister *> orig_wire_to_stage_wire_;
  map<IRegister *, IRegister *> stage_wire_to_orig_wire_;
};

}  // namespace pipeline
}  // namespace opt
}  // namespace iroha

#endif  // _opt_pipeline_pipeliner_h_
