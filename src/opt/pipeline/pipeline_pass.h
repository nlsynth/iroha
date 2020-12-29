// -*- C++ -*-
#ifndef _opt_pipeline_pipeline_pass_h_
#define _opt_pipeline_pipeline_pass_h_

#include "opt/pass.h"

namespace iroha {
namespace opt {
namespace loop {
class LoopBlock;
}  // namespace loop
namespace pipeline {

class PipelinePass : public Pass {
 public:
  virtual ~PipelinePass();

  static Pass *Create();

 private:
  virtual bool ApplyForTable(const string &key, ITable *table);

  bool CheckWriteConflict(loop::LoopBlock *lb);
};

}  // namespace pipeline
}  // namespace opt
}  // namespace iroha

#endif  // _opt_pipeline_pipeline_pass_h_
