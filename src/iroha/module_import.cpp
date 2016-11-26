#include "iroha/module_import.h"

namespace iroha {

ModuleImport::ModuleImport(IModule *mod, const string &fn)
  : mod_(mod), fn_(fn) {
}

ModuleImport::~ModuleImport() {
}

const string &ModuleImport::GetFileName() {
  return fn_;
}

}  // namespace iroha
