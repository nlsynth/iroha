// -*- C++ -*-
#ifndef _opt_unroll_unroll_phase_h_
#define _opt_unroll_unroll_phase_h_

#include "opt/phase.h"

namespace iroha {
namespace opt {
namespace unroll {

class UnrollPhase : public Phase {
public:
  virtual ~UnrollPhase();

  static Phase *Create();
};

}  // namespace unroll
}  // namespace opt
}  // namespace iroha

#endif  // _opt_unroll_unroll_phase_h_
