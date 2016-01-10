// -*- C++ -*-
#ifndef _opt_phase_h_
#define _opt_phase_h_

#include "iroha/common.h"

namespace iroha {
namespace opt {

class Phase {
public:
  virtual ~Phase();

  virtual bool ApplyForDesign(IDesign *design);

protected:
  virtual bool ApplyForModule(IModule *module);
  virtual bool ApplyForTable(ITable *table);
};

}  // namespace opt
}  // namespace iroha

#endif  // _opt_phase_h_
