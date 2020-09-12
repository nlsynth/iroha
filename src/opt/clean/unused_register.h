// -*- C++ -*-
#ifndef _opt_clean_unused_register_h_
#define _opt_clean_unused_register_h_

#include "opt/pass.h"

namespace iroha {
namespace opt {
namespace clean {

class CleanUnusedRegPhase : public Pass {
 public:
  virtual ~CleanUnusedRegPhase();

  static Pass *Create();

 private:
  virtual bool ApplyForTable(const string &key, ITable *table);
};

}  // namespace clean
}  // namespace opt
}  // namespace iroha

#endif  // _opt_clean_unused_register_h_
