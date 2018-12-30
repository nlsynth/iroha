// -*- C++ -*-
#ifndef _opt_sched_wire_h_
#define _opt_sched_wire_h_

#include "opt/sched/common.h"

namespace iroha {
namespace opt {
namespace sched {

class Wire {
public:
  Wire(ITable *table, DelayInfo *delay_info, DebugAnnotation *annotation);
  virtual ~Wire();
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

#endif  // _opt_sched_wire_h_
