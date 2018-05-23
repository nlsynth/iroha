#include "opt/clean/empty_table.h"

#include "iroha/i_design.h"
#include "iroha/resource_class.h"

namespace iroha {
namespace opt {
namespace clean {

CleanEmptyTablePhase::~CleanEmptyTablePhase() {
}

Phase *CleanEmptyTablePhase::Create() {
  return new CleanEmptyTablePhase();
}

bool CleanEmptyTablePhase::ApplyForDesign(IDesign *design) {
  bool ok = ApplyForAllModules("scan", design);
  if (!ok) {
    return false;
  }
  return ApplyForAllModules("process", design);
}

bool CleanEmptyTablePhase::ApplyForModule(const string &key, IModule *module) {
  if (key == "scan") {
    return Phase::ApplyForModule("", module);
  }
  vector<ITable *> tables;
  for (auto *table : module->tables_) {
    bool has_ref = false;
    if (has_foreign_reg_.find(table) !=
	has_foreign_reg_.end()) {
      has_ref = true;
    }
    if (!IsEmpty(table) || has_ref) {
      tables.push_back(table);
    }
  }
  module->tables_ = tables;
  return true;
}

bool CleanEmptyTablePhase::ApplyForTable(const string &key, ITable *table) {
  for (IResource *res : table->resources_) {
    if (resource::IsForeignRegister(*(res->GetClass()))) {
      IRegister *reg = res->GetForeignRegister();
      has_foreign_reg_.insert(reg->GetTable());
    }
  }
  return true;
}

bool CleanEmptyTablePhase::IsEmpty(ITable *tab) {
  for (IResource *res : tab->resources_) {
    if (!resource::IsTransition(*(res->GetClass()))) {
      return false;
    }
  }
  return true;
}

}  // namespace clean
}  // namespace opt
}  // namespace iroha
