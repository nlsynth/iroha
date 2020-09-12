// -*- C++ -*-
#ifndef _opt_clean_pseudo_resource_h_
#define _opt_clean_pseudo_resource_h_

#include "opt/pass.h"

namespace iroha {
namespace opt {
namespace clean {

class CleanPseudoResourcePhase : public Phase {
 public:
  virtual ~CleanPseudoResourcePhase();

  static Phase *Create();

 private:
  virtual bool ApplyForTable(const string &key, ITable *table);
};

}  // namespace clean
}  // namespace opt
}  // namespace iroha

#endif  // _opt_clean_pseudo_resource_h_
