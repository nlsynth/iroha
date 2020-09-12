// -*- C++ -*-
//
// Kills unreachable states.
//
#ifndef _opt_clean_unreachable_state_h_
#define _opt_clean_unreachable_state_h_

#include "opt/pass.h"

namespace iroha {
namespace opt {
namespace clean {

class CleanUnreachableStatePhase : public Phase {
 public:
  virtual ~CleanUnreachableStatePhase();

  static Phase *Create();

 private:
  virtual bool ApplyForTable(const string &key, ITable *table);
};

}  // namespace clean
}  // namespace opt
}  // namespace iroha

#endif  // _opt_clean_unreachable_state_h_
