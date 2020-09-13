// -*- C++ -*-
#ifndef _opt_pipeline_pipeliner_h_
#define _opt_pipeline_pipeliner_h_

#include "opt/common.h"

namespace iroha {
namespace opt {
namespace loop {
class LoopBlock;
}  // namespace loop
namespace pipeline {

class Pipeliner {
 public:
  Pipeliner(ITable *tab, loop::LoopBlock *lb);

  bool Pipeline();

 private:
  ITable *tab_;
  loop::LoopBlock *lb_;
  OptimizerLog *opt_log_;
};

}  // namespace pipeline
}  // namespace opt
}  // namespace iroha

#endif  // _opt_pipeline_pipeliner_h_
