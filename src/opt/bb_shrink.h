// -*- C++ -*-
//
// Moves insns in a basic block to earlier cycles.
//
#ifndef _opt_bb_shrink_h_
#define _opt_bb_shrink_h_

#include "opt/phase.h"

namespace iroha {
namespace opt {

class BB;

class BBShrinkPhase : public Phase {
public:
  virtual ~BBShrinkPhase();

  static Phase *Create();

private:
  virtual bool ApplyForTable(ITable *table);
  void ShrinkBB(BB *bb);
};

}  // namespace opt
}  // namespace iroha

#endif  // _opt_bb_shrink_h_
