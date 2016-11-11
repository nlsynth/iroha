#include "design/importer.h"

#include "design/design_util.h"
#include "design/module_copier.h"
#include "iroha/i_design.h"
#include "iroha/iroha.h"
#include "iroha/module_import.h"

namespace iroha {

Importer::Importer(IDesign *design) : design_(design) {
}

void Importer::Import(IDesign *design) {
  Importer importer(design);
  importer.Resolve();
}

void Importer::Resolve() {
  IModule *root = DesignUtil::GetRootModule(design_);
  TraverseModule(root);
}

void Importer::TraverseModule(IModule *mod) {
  ModuleImport *mi = mod->GetModuleImport();
  if (mi != nullptr) {
    ProcessImport(mod);
    mod->SetModuleImport(nullptr);
  }
  vector<IModule *> child_mods = DesignUtil::GetChildModules(mod);
  for (IModule *child : child_mods) {
    TraverseModule(child);
  }
}

void Importer::ProcessImport(IModule *mod) {
  ModuleImport *mi = mod->GetModuleImport();
  const string &fn = mi->GetFileName();
  IDesign *design = Iroha::ReadDesignFromFile(fn);
  IModule *root = DesignUtil::GetRootModule(design);
  ModuleCopier::CopyModule(root, mod);
  std::unique_ptr<IDesign> deleter(design);
}

}  // namespace iroha
