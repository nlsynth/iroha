#include "opt/ssa/ssa.h"

#include "opt/ssa/phi_cleaner.h"
#include "opt/ssa/ssa_converter.h"

namespace iroha {
namespace opt {
namespace ssa {

SSAConverterPass::~SSAConverterPass() {}

Pass *SSAConverterPass::Create() { return new SSAConverterPass(); }

bool SSAConverterPass::ApplyForTable(const string &key, ITable *table) {
  SSAConverter converter(table, annotation_);
  converter.Perform();
  return true;
}

PhiCleanerPass::~PhiCleanerPass() {}

Pass *PhiCleanerPass::Create() { return new PhiCleanerPass(); }

bool PhiCleanerPass::ApplyForTable(const string &key, ITable *table) {
  PhiCleaner cleaner(table, annotation_);
  cleaner.Perform();
  return true;
}

}  // namespace ssa
}  // namespace opt
}  // namespace iroha
