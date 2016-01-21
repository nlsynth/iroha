#include "opt/ssa/ssa.h"

#include "opt/ssa/ssa_converter.h"

namespace iroha {
namespace opt {
namespace ssa {

SSAConverterPhase::~SSAConverterPhase() {
}

Phase *SSAConverterPhase::Create() {
  return new SSAConverterPhase();
}

bool SSAConverterPhase::ApplyForTable(ITable *table) {
  SSAConverter converter(table, annotation_);
  converter.Perform();
  return true;
}

}  // namespace ssa
}  // namespace opt
}  // namespace iroha
