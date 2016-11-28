#include "iroha/module_import.h"

namespace iroha {

ModuleImportTap::ModuleImportTap()
  : module_id(-1), table_id(-1), resource_id(-1) {
}

ModuleImport::ModuleImport(IModule *mod, const string &fn)
  : mod_(mod), fn_(fn) {
}

ModuleImport::~ModuleImport() {
}

const string &ModuleImport::GetFileName() {
  return fn_;
}

}  // namespace iroha
