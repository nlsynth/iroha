// -*- C++ -*-
#ifndef _opt_pipeline_pipeline_phase_h_
#define _opt_pipeline_pipeline_phase_h_

#include "opt/pass.h"

namespace iroha {
namespace opt {
namespace pipeline {

class PipelinePhase : public Pass {
 public:
  virtual ~PipelinePhase();

  static Pass *Create();

 private:
  virtual bool ApplyForTable(const string &key, ITable *table);
};

}  // namespace pipeline
}  // namespace opt
}  // namespace iroha

#endif  // _opt_unroll_unroll_phase_h_
