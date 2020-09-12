// -*- C++ -*-
#ifndef _opt_pass_h_
#define _opt_pass_h_

#include "opt/common.h"

namespace iroha {
namespace opt {

class Pass {
 public:
  Pass();
  virtual ~Pass();

  // Following 3 methods must be called after the construction.
  void SetName(const string &name);
  void SetOptimizer(Optimizer *opt);
  void SetOptimizerLog(OptimizerLog *opt_log);

  bool Apply(IDesign *design);

 protected:
  // Default implementation just traverses modules and tables.
  // Each Pass can implement its own strategies to process modules and tables
  // like skipping or multi pass traversal.
  virtual bool ApplyForDesign(IDesign *design);
  virtual bool ApplyForModule(const string &key, IModule *module);
  virtual bool ApplyForTable(const string &key, ITable *table);

  bool ApplyForAllModules(const string &key, IDesign *design);
  void OutputPassHeader(const string &msg);

  Optimizer *optimizer_;
  // Optimizers can assume this is not null.
  OptimizerLog *opt_log_;
  string name_;
};

}  // namespace opt
}  // namespace iroha

#endif  // _opt_pass_h_
