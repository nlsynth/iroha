// WIP.
#include "opt/wire/wire.h"

#include "design/resource_attr.h"
#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"
#include "opt/wire/resource_share.h"

namespace iroha {
namespace opt {
namespace wire {

Wire::Wire(ITable *table, DebugAnnotation *annotation)
  : Scaffold(table, annotation) {
  resource_share_.reset(new ResourceShare(table));
}

Wire::~Wire() {
}

bool Wire::Perform() {
  resource_share_->Scan();
  return true;
}

}  // namespace wire
}  // namespace opt
}  // namespace iroha
