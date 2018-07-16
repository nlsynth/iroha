// -*- C++ -*-
#ifndef _opt_wire_alloc_resource_h_
#define _opt_wire_alloc_resource_h_

#include "opt/wire/wire_insn.h"

namespace iroha {
namespace opt {
namespace wire {

class AllocResourcePhase : public Phase {
public:
  virtual ~AllocResourcePhase();

  static Phase *Create();

private:
  virtual bool ApplyForTable(const string &key, ITable *table);
};

class AllocResource : public WireInsn {
public:
  AllocResource(ITable *table, DebugAnnotation *annotation);
  virtual ~AllocResource();

private:
  virtual bool CanUseResourceInState(IState *st, IResource *resource,
				     MoveStrategy *ms);
  virtual IInsn *MayCopyInsnForState(IState *st, IInsn *insn,
				     MoveStrategy *ms);
  IResource *CopyResource(IResource *src);
  IResource *FindCompatibleResource(IState *st, IResource *res);
};


}  // namespace wire
}  // namespace opt
}  // namespace iroha

#endif  // _opt_wire_alloc_resource_h_

