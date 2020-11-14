// -*- C++ -*-
#ifndef _opt_branch_elimination_branch_elimination_pass_h_
#define _opt_branch_elimination_branch_elimination_pass_h_

#include "opt/pass.h"

namespace iroha {
namespace opt {
namespace branch_elimination {

class BranchElminationPass : public Pass {
 public:
  virtual ~BranchElminationPass();

  static Pass *Create();

 private:
  virtual bool ApplyForTable(const string &key, ITable *table);
};

}  // namespace branch_elimination
}  // namespace opt
}  // namespace iroha

#endif  // _opt_branch_elimination_branch_elimination_pass_h_
