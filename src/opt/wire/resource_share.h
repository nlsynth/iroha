// -*- C++ -*-
#ifndef _opt_wire_resource_share_h_
#define _opt_wire_resource_share_h_

#include "opt/wire/common.h"

namespace iroha {
namespace opt {
namespace wire {

// Manages resource usages in a table.
class ResourceShare {
public:
  ResourceShare(ITable *tab);

  void Scan();

private:
  ITable *tab_;
};

}  // namespace wire
}  // namespace opt
}  // namespace iroha

#endif  // _opt_wire_resource_share_h_

