// -*- C++ -*-
#ifndef _opt_pipeline_insn_condition_h_
#define _opt_pipeline_insn_condition_h_

#include "opt/common.h"

namespace iroha {
namespace opt {
namespace pipeline {

// Finds conditional executions in a pipeline.
class InsnCondition {
 public:
  InsnCondition(loop::LoopBlock *lb);

  bool Build(OptimizerLog *log);

 private:
  ITable *tab_;
  loop::LoopBlock *lb_;
};

}  // namespace pipeline
}  // namespace opt
}  // namespace iroha

#endif  // _opt_pipeline_insn_condition_h_
