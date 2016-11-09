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
  ITable *src_tab = DesignUtil::FindTableById(src_mod,
					      res->GetTable()->GetId());
  IResource *src_res = DesignUtil::FindResourceById(src_tab,
						    res->GetId());
  // TODO copy channels too.
  ITable *callee_table = src_res->GetCalleeTable();
  if (callee_table != nullptr) {
    SetCalleeTable(callee_table, res);
  }
  IRegister *foreign_register = src_res->GetForeignRegister();
  if (foreign_register != nullptr) {
    SetForeignRegister(foreign_register, res);
  }
  IResource *shared_reg = src_res->GetSharedRegister();
  if (shared_reg != nullptr) {
    SetSharedRegister(shared_reg, res);
  }
}

void ResourceCopier::SetCalleeTable(ITable *callee_table, IResource *res) {
  IModule *mod = callee_table->GetModule();
  IModule *new_mod = module_map_[mod];
  ITable *new_tab =
    DesignUtil::FindTableById(new_mod, callee_table->GetId());
  res->SetCalleeTable(new_tab);
}

void ResourceCopier::SetForeignRegister(IRegister *foreign_register,
					IResource *res) {
  IModule *mod = foreign_register->GetTable()->GetModule();
  IModule *new_mod = module_map_[mod];
  ITable *new_tab =
    DesignUtil::FindTableById(new_mod,
			      foreign_register->GetTable()->GetId());
  IRegister *new_reg = DesignUtil::FindRegisterById(new_tab,
						    foreign_register->GetId());
  res->SetForeignRegister(new_reg);
}

void ResourceCopier::SetSharedRegister(IResource *shared_reg,
				       IResource *res) {
  IModule *mod = shared_reg->GetTable()->GetModule();
  IModule *new_mod = module_map_[mod];
  ITable *new_tab =
    DesignUtil::FindTableById(new_mod,
			      shared_reg->GetTable()->GetId());
  IResource *new_res = DesignUtil::FindResourceById(new_tab,
						    shared_reg->GetId());
  res->SetSharedRegister(new_res);
}

}  // namespace iroha
