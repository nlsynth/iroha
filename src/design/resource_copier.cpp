#include "design/resource_copier.h"

#include "design/design_util.h"

namespace iroha {

ResourceCopier::ResourceCopier(IModule *root,
			       map<IModule *, IModule *> &module_map)
  : new_root_mod_(root), module_map_(module_map) {
}

void ResourceCopier::Copy() {
  for (auto it : module_map_) {
    reverse_map_[it.second] = it.first;
  }
  TraverseModule(new_root_mod_);
}

void ResourceCopier::TraverseModule(IModule *mod) {
  for (ITable *tab : mod->tables_) {
    ProcessTable(tab);
  }
  vector<IModule *> child_mods = DesignUtil::GetChildModules(mod);
  for (IModule *mod : child_mods) {
    TraverseModule(mod);
  }
}

void ResourceCopier::ProcessTable(ITable *tab) {
  for (IResource *res : tab->resources_) {
    ProcessResource(res);
  }
}

void ResourceCopier::ProcessResource(IResource *res) {
  IModule *mod = res->GetTable()->GetModule();
  IModule *src_mod = reverse_map_[mod];
  IResource *src_res = FindResource(src_mod, res->GetTable()->GetId(),
				    res->GetId());
  // TODO copy other various objects too.
  ITable *callee_table = src_res->GetCalleeTable();
  if (callee_table != nullptr) {
    SetCalleeTable(callee_table, res);
  }
}

void ResourceCopier::SetCalleeTable(ITable *callee_table, IResource *res) {
  IModule *mod = callee_table->GetModule();
  IModule *new_mod = module_map_[mod];
  ITable *new_tab = FindTable(new_mod, callee_table->GetId());
  res->SetCalleeTable(new_tab);
}

IResource *ResourceCopier::FindResource(IModule *mod, int tab_id, int res_id) {
  ITable *tab = FindTable(mod, tab_id);
  if (tab == nullptr) {
    return nullptr;
  }
  for (IResource *res : tab->resources_) {
    if (res->GetId() == res_id) {
      return res;
    }
  }
  return nullptr;
}

ITable *ResourceCopier::FindTable(IModule *mod, int tab_id) {
  for (ITable *tab : mod->tables_) {
    if (tab->GetId() == tab_id) {
      return tab;
    }
  }
  return nullptr;
}

}  // namespace iroha
