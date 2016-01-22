#include "opt/ssa/ssa_converter.h"

#include "design/design_tool.h"
#include "design/design_util.h"
#include "iroha/i_design.h"
#include "iroha/logging.h"
#include "iroha/resource_class.h"
#include "opt/bb_set.h"
#include "opt/data_flow.h"
#include "opt/dominator_tree.h"
#include "opt/opt_util.h"
#include "opt/ssa/phi_builder.h"
#include "opt/ssa/phi_injector.h"

namespace iroha {
namespace opt {
namespace ssa {

SSAConverter::SSAConverter(ITable *table, DebugAnnotation *annotation)
  : table_(table), annotation_(annotation) {
}

SSAConverter::~SSAConverter() {
}

void SSAConverter::Perform() {
  PhiInjector injector(table_, annotation_);
  injector.Perform();

  PhiBuilder phi_builder(table_, annotation_);
  phi_builder.Perform();
}

}  // namespace ssa
}  // namespace opt
}  // namespace iroha
