// -*- C++ -*-
//
// Moves insns in a basic block to earlier cycles.
//
#ifndef _opt_bb_shrink_h_
#define _opt_bb_shrink_h_

#include "opt/phase.h"

namespace iroha {
namespace opt {

class BBShrinkPhase : public Phase {
public:
  virtual ~BBShrinkPhase();

  static Phase *Create();

private:
  virtual bool ApplyForTable(ITable *table);
};

class BBShrink {
public:
  BBShrink(ITable *table,  DebugAnnotation *annotation);
  bool Perform();

private:
  void ShrinkBB(BB *bb);
  IState *GetNextIfDead(IState *st);

  ITable *table_;
  DebugAnnotation *annotation_;
  IResource *transition_;
  set<IState *> dead_st_;
};

}  // namespace opt
}  // namespace iroha

#endif  // _opt_bb_shrink_h_
