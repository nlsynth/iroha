// -*- C++ -*-
#ifndef _opt_clean_pseudo_resource_h_
#define _opt_clean_pseudo_resource_h_

#include "opt/pass.h"

namespace iroha {
namespace opt {
namespace clean {

class CleanPseudoResourcePass : public Pass {
 public:
  virtual ~CleanPseudoResourcePass();

  static Pass *Create();

 private:
  virtual bool ApplyForTable(const string &key, ITable *table);
};

}  // namespace clean
}  // namespace opt
}  // namespace iroha

#endif  // _opt_clean_pseudo_resource_h_
