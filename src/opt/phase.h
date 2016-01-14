// -*- C++ -*-
#ifndef _opt_phase_h_
#define _opt_phase_h_

#include "iroha/common.h"

namespace iroha {
namespace opt {

class DebugAnnotation;

class Phase {
public:
  Phase();
  virtual ~Phase();

  virtual bool ApplyForDesign(IDesign *design);

protected:
  virtual bool ApplyForModule(IModule *module);
  virtual bool ApplyForTable(ITable *table);

  DebugAnnotation *annotation_;
};

}  // namespace opt
}  // namespace iroha

#endif  // _opt_phase_h_
