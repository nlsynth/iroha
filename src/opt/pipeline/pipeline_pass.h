// -*- C++ -*-
#ifndef _opt_pipeline_pipeline_pass_h_
#define _opt_pipeline_pipeline_pass_h_

#include "opt/pass.h"

namespace iroha {
namespace opt {
namespace pipeline {

class PipelinePass : public Pass {
 public:
  virtual ~PipelinePass();

  static Pass *Create();

 private:
  virtual bool ApplyForTable(const string &key, ITable *table);
};

}  // namespace pipeline
}  // namespace opt
}  // namespace iroha

#endif  // _opt_unroll_pipeline_pass_h_
