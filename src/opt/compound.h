// -*- C++ -*-
//
// Pre-defined phase sets.
//
#ifndef _opt_compound_h_
#define _opt_compound_h_

#include "opt/phase.h"

namespace iroha {
namespace opt {

class CompoundPhase : public Phase {
public:
  virtual ~CompoundPhase();

  static void Init();
  static Phase *Create();

private:
  virtual bool ApplyForDesign(IDesign *design);
};

}  // namespace opt
}  // namespace iroha

#endif  // _opt_compound_h_
