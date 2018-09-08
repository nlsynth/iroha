// -*- C++ -*-
#ifndef _opt_wire_alloc_resource_h_
#define _opt_wire_alloc_resource_h_

#include "opt/wire/scaffold.h"

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

class AllocResource : public Scaffold {
public:
  AllocResource(ITable *table, DebugAnnotation *annotation);
  virtual ~AllocResource();
  bool Perform();

private:
};

}  // namespace wire
}  // namespace opt
}  // namespace iroha

#endif  // _opt_wire_alloc_resource_h_

