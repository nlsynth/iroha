#include "builder/tree_builder.h"

#include "builder/exp_builder.h"
#include "builder/reader.h"
#include "iroha/i_design.h"

namespace iroha {
namespace builder {

TreeBuilder::TreeBuilder(IDesign *design, ExpBuilder *builder)
  : design_(design), builder_(builder) {
}

void TreeBuilder::AddSubModule(const string &name, IResource *res) {
  sub_module_names_[res] = name;
}

void TreeBuilder::AddParentModule(const string &name, IModule *mod) {
  parent_module_names_[mod] = name;
}

bool TreeBuilder::Resolve() {
  map<string, IModule *> module_names;
  for (IModule *mod : design_->modules_) {
    module_names[mod->GetName()] = mod;
  }
  for (auto p : sub_module_names_) {
    IModule *mod = module_names[p.second];
    if (mod == nullptr) {
      builder_->SetError() << "unknown module: " << p.second;
      return false;
    }
    IResource *res = p.first;
    res->SetModule(mod);
  }
  for (auto p : parent_module_names_) {
    IModule *mod = module_names[p.second];
    if (mod == nullptr) {
      builder_->SetError() << "unknown module: " << p.second;
      return false;
    }
    IModule *cmod = p.first;
    cmod->SetParentModule(mod);
  }
  return true;
}

}  // namespace builder
}  // namespace iroha
