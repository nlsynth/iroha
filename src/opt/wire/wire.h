// -*- C++ -*-
#ifndef _opt_wire_wire_h_
#define _opt_wire_wire_h_

#include "opt/wire/common.h"

namespace iroha {
namespace opt {
namespace wire {

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

  unique_ptr<ResourceShare> resource_share_;
  unique_ptr<DataPathSet> data_path_set_;
};

}  // namespace wire
}  // namespace opt
}  // namespace iroha

#endif  // _opt_wire_wire_h_
