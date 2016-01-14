#include "opt/phase.h"

#include "iroha/i_design.h"

namespace iroha {
namespace opt {

Phase::Phase() : annotation_(nullptr) {
}

Phase::~Phase() {
}

bool Phase::ApplyForDesign(IDesign *design) {
  annotation_ = design->GetDebugAnnotation();
  bool all_ok = true;
  for (auto *mod : design->modules_) {
    all_ok &= ApplyForModule(mod);
  }
  return all_ok;
}

bool Phase::ApplyForModule(IModule *module) {
  bool all_ok = true;
  for (auto *table : module->tables_) {
    all_ok &= ApplyForTable(table);
  }
  return all_ok;
}

bool Phase::ApplyForTable(ITable *table) {
  return true;
}

}  // namespace opt
}  // namespace iroha
