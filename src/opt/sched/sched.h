// -*- C++ -*-
#ifndef _opt_sched_sched_h_
#define _opt_sched_sched_h_

#include "opt/sched/common.h"

namespace iroha {
namespace opt {
namespace sched {

class Sched {
public:
  Sched(ITable *table, DelayInfo *delay_info, DebugAnnotation *annotation);
  virtual ~Sched();
  bool Perform();

private:
  void IterateScheduling();

  ITable *table_;
  DelayInfo *delay_info_;
  DebugAnnotation *annotation_;
  unique_ptr<BBSet> bset_;

  unique_ptr<DataPathSet> data_path_set_;
};

}  // namespace sched
}  // namespace opt
}  // namespace iroha

#endif  // _opt_sched_sched_h_
