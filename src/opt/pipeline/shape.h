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

  // pipeline macro stage, loop index.
  vector<pair<int, int>> &GetPipelineLocation();
  vector<int> GetPipeLineIndexRange(int s, int e);

 private:
  StageScheduler *ssch_;
  vector<pair<int, int>> locs_;
};

}  // namespace pipeline
}  // namespace opt
}  // namespace iroha

#endif  // _opt_pipeline_shape_h_
