// -*- C++ -*-
#ifndef _opt_sched_sched_pass_h_
#define _opt_sched_sched_pass_h_

#include "opt/common.h"
#include "opt/pass.h"

namespace iroha {
namespace opt {
namespace sched {

class SchedPass : public Pass {
 public:
  virtual ~SchedPass();

  static Pass *Create();

 private:
  virtual bool ApplyForDesign(IDesign *design);
  virtual bool ApplyForTable(const string &key, ITable *table);

  std::unique_ptr<DelayInfo> delay_info_;
};

}  // namespace sched
}  // namespace opt
}  // namespace iroha

#endif  // _opt_sched_sched_pass_h_
