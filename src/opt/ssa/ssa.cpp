#include "opt/ssa/ssa.h"

#include "opt/ssa/phi_cleaner.h"
#include "opt/ssa/ssa_converter.h"

namespace iroha {
namespace opt {
namespace ssa {

SSAConverterPhase::~SSAConverterPhase() {}

Pass *SSAConverterPhase::Create() { return new SSAConverterPhase(); }

bool SSAConverterPhase::ApplyForTable(const string &key, ITable *table) {
  SSAConverter converter(table, annotation_);
  converter.Perform();
  return true;
}

PhiCleanerPhase::~PhiCleanerPhase() {}

Pass *PhiCleanerPhase::Create() { return new PhiCleanerPhase(); }

bool PhiCleanerPhase::ApplyForTable(const string &key, ITable *table) {
  PhiCleaner cleaner(table, annotation_);
  cleaner.Perform();
  return true;
}

}  // namespace ssa
}  // namespace opt
}  // namespace iroha
