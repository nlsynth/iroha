// -*- C++ -*-
#ifndef _opt_phase_h_
#define _opt_phase_h_

#include "opt/common.h"

namespace iroha {
namespace opt {

class Phase {
public:
  Phase();
  virtual ~Phase();

  void SetAnnotation(DebugAnnotation *annotation);
  virtual bool ApplyForDesign(IDesign *design);

protected:
  virtual bool ApplyForModule(const string &key, IModule *module);
  virtual bool ApplyForTable(const string &key, ITable *table);

  bool ApplyForAllModules(const string &key, IDesign *design);

  DebugAnnotation *annotation_;
};

}  // namespace opt
}  // namespace iroha

#endif  // _opt_phase_h_
