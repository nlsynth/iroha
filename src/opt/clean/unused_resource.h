// -*- C++ -*-
#ifndef _opt_clean_unused_resource_h_
#define _opt_clean_unused_resource_h_

#include "opt/phase.h"

namespace iroha {
namespace opt {
namespace clean {

class CleanUnusedResourcePhase : public Phase {
public:
  virtual ~CleanUnusedResourcePhase();

  static Phase *Create();

private:
  virtual bool ApplyForTable(ITable *table);
};

}  // namespace clean
}  // namespace opt
}  // namespace iroha

#endif  // _opt_clean_unused_resource_h_

