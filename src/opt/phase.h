// -*- C++ -*-
#ifndef _opt_phase_h_
#define _opt_phase_h_

#include "iroha/common.h"

namespace iroha {

class Phase {
public:
  virtual ~Phase();
  virtual const string &GetName() const;

  static void Register(Phase *phase);

  virtual bool ApplyForDesign(IDesign *design);

protected:
  virtual bool ApplyForModule(IModule *module);
  virtual bool ApplyForTable(ITable *table);
};

}  // namespace iroha

#endif  // _opt_phase_h_
