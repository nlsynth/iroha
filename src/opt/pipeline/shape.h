// -*- C++ -*-
#ifndef _opt_pipeline_shape_h_
#define _opt_pipeline_shape_h_

#include "opt/common.h"

namespace iroha {
namespace opt {
namespace pipeline {

class StageScheduler;

class Shape {
 public:
  Shape(StageScheduler *ssch);

  vector<pair<int, int>> GetPipelineLocation();

 private:
  StageScheduler *ssch_;
};

}  // namespace pipeline
}  // namespace opt
}  // namespace iroha

#endif  // _opt_pipeline_shape_h_
