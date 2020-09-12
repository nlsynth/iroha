// -*- C++ -*-
#ifndef _opt_sched_data_path_set_h_
#define _opt_sched_data_path_set_h_

#include "opt/sched/common.h"

namespace iroha {
namespace opt {
namespace sched {

class DataPathSet {
 public:
  DataPathSet();
  ~DataPathSet();

  void Build(BBSet *bbs);
  void SetDelay(DelayInfo *dinfo);
  void Dump(OptimizerLog *an);
  map<int, BBDataPath *> &GetBBPaths();
  BBSet *GetBBSet();
  VirtualResourceSet *GetVirtualResourceSet();

 private:
  BBSet *bbs_;
  std::unique_ptr<SchedBlockSet> sbs_;
  // bb_id to BBDataPath.
  map<int, BBDataPath *> data_paths_;
  std::unique_ptr<VirtualResourceSet> vres_set_;
};

}  // namespace sched
}  // namespace opt
}  // namespace iroha

#endif  // _opt_sched_data_path_h_
