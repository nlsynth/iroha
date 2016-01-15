// -*- C++ -*-
#ifndef _opt_wire_insn_h_
#define _opt_wire_insn_h_

#include "opt/phase.h"

namespace iroha {
namespace opt {

class WireInsnPhase : public Phase {
public:
  virtual ~WireInsnPhase();

  static Phase *Create();

private:
  virtual bool ApplyForTable(ITable *table);
};

class WireInsn {
public:
  WireInsn(ITable *table, DebugAnnotation *annotation);
  ~WireInsn();
  bool Perform();

private:
  ITable *table_;
  DebugAnnotation *annotation_;
  BBSet *bbs_;
  DataFlow *data_flow_;
};
  
}  // namespace opt
}  // namespace iroha

#endif  // _opt_wire_insn_h_
