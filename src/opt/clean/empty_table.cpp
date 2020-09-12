#include "opt/clean/empty_table.h"

#include "iroha/i_design.h"
#include "iroha/resource_class.h"

namespace iroha {
namespace opt {
namespace clean {

CleanEmptyTablePass::~CleanEmptyTablePass() {}

Pass *CleanEmptyTablePass::Create() { return new CleanEmptyTablePass(); }

bool CleanEmptyTablePass::ApplyForDesign(IDesign *design) {
  bool ok = ApplyForAllModules("scan", design);
  if (!ok) {
    return false;
  }
  return ApplyForAllModules("process", design);
}

bool CleanEmptyTablePass::ApplyForModule(const string &key, IModule *module) {
  if (key == "scan") {
    return Pass::ApplyForModule("", module);
  }
  vector<ITable *> tables;
  for (auto *table : module->tables_) {
    if (!IsEmpty(table)) {
      tables.push_back(table);
    }
  }
  module->tables_ = tables;
  return true;
}

bool CleanEmptyTablePass::ApplyForTable(const string &key, ITable *table) {
  return true;
}

bool CleanEmptyTablePass::IsEmpty(ITable *tab) {
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
