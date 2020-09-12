// -*- C++ -*-
#ifndef _opt_sched_sched_phase_h_
#define _opt_sched_sched_phase_h_

#include "opt/common.h"
#include "opt/pass.h"

namespace iroha {
namespace opt {
namespace sched {

class SchedPhase : public Phase {
 public:
  virtual ~SchedPhase();

  static Phase *Create();

 private:
  virtual bool ApplyForDesign(IDesign *design);
  virtual bool ApplyForTable(const string &key, ITable *table);

  std::unique_ptr<DelayInfo> delay_info_;
};

}  // namespace sched
}  // namespace opt
}  // namespace iroha

#endif  // _opt_sched_sched_phase_h_
