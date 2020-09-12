// -*- C++ -*-
#ifndef _opt_unroll_unroll_phase_h_
#define _opt_unroll_unroll_phase_h_

#include "opt/pass.h"

namespace iroha {
namespace opt {
namespace unroll {

class UnrollPhase : public Pass {
 public:
  virtual ~UnrollPhase();

  static Pass *Create();

 private:
  virtual bool ApplyForTable(const string &key, ITable *table);
};

}  // namespace unroll
}  // namespace opt
}  // namespace iroha

#endif  // _opt_unroll_unroll_phase_h_
