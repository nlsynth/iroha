// -*- C++ -*-
#ifndef _opt_pipeline_scheduled_shape_h_
#define _opt_pipeline_scheduled_shape_h_

#include "opt/common.h"

namespace iroha {
namespace opt {
namespace pipeline {

class StageScheduler;

class ScheduledShape {
 public:
  ScheduledShape(StageScheduler *ssch);
  ~ScheduledShape();

  // pipeline macro stage, loop index.
  vector<pair<int, int>> &GetPipelineLocation();
  vector<pair<int, int>> GetPipeLineIndexRange(int s, int e);

 private:
  StageScheduler *ssch_;
  vector<pair<int, int>> locs_;
};

}  // namespace pipeline
}  // namespace opt
}  // namespace iroha

#endif  // _opt_pipeline_scheduled_shape_h_
