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

  void SetName(const string &name);
  void SetOptimizer(Optimizer *opt);
  // Optional. Set when debug output is enabled.
  void SetAnnotation(DebugAnnotation *annotation);
  bool Apply(IDesign *design);

protected:
  // Default implementation just traverses modules and tables.
  // Each phase can implement its own strategies to process modules and tables
  // like skipping or multi pass traversal.
  virtual bool ApplyForDesign(IDesign *design);
  virtual bool ApplyForModule(const string &key, IModule *module);
  virtual bool ApplyForTable(const string &key, ITable *table);

  bool ApplyForAllModules(const string &key, IDesign *design);
  void OutputPhaseHeader(const string &msg);

  Optimizer *optimizer_;
  DebugAnnotation *annotation_;
  string name_;
};

}  // namespace opt
}  // namespace iroha

#endif  // _opt_phase_h_
