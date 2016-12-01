#include "iroha/module_import.h"

#include "iroha/stl_util.h"

namespace iroha {

ModuleImportTap::ModuleImportTap() : resource(nullptr) {
}

ModuleImport::ModuleImport(IModule *mod, const string &fn)
  : mod_(mod), fn_(fn) {
}

ModuleImport::~ModuleImport() {
  STLDeleteValues(&taps_);
}

const string &ModuleImport::GetFileName() {
  return fn_;
}

}  // namespace iroha
