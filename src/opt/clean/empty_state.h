// -*- C++ -*-
//
// Kills empty states.
//
#ifndef _opt_clean_empty_state_h_
#define _opt_clean_empty_state_h_

#include "opt/pass.h"

namespace iroha {
namespace opt {
namespace clean {

class CleanEmptyStatePass : public Pass {
 public:
  virtual ~CleanEmptyStatePass();

  static Pass *Create();

 private:
  virtual bool ApplyForTable(const string &key, ITable *table);
};

class CleanEmptyState {
 public:
  CleanEmptyState(ITable *table, OptimizerLog *opt_log);
  bool Perform();

 private:
  IState *GetNextIfDead(IState *st);
  bool IsEmptyState(IState *st);

  ITable *table_;
  OptimizerLog *opt_log_;
  IResource *transition_;
  set<IState *> dead_st_;
};

}  // namespace clean
}  // namespace opt
}  // namespace iroha

#endif  // _opt_clean_empty_state_h_
