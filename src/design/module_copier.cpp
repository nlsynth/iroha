#include "design/module_copier.h"

#include "design/design_util.h"
#include "design/resource_copier.h"
#include "design/table_copier.h"

namespace iroha {

ModuleCopier::ModuleCopier(IModule *src, IModule *empty_mod)
  : src_root_mod_(src), new_root_mod_(empty_mod) {
}

void ModuleCopier::CopyModule(IModule *src, IModule *empty_mod) {
  unique_ptr<ModuleCopier> copier(new ModuleCopier(src, empty_mod));
  copier->Copy();
}

void ModuleCopier::Copy() {
  for (IModule *m : new_root_mod_->GetDesign()->modules_) {
    mod_id_in_new_design_[m->GetId()] = m;
  }
  CopyRec(src_root_mod_, new_root_mod_);
  module_map_[src_root_mod_] = new_root_mod_;
  ResourceCopier copier(new_root_mod_, module_map_);
  copier.Copy();
}

void ModuleCopier::CopyRec(IModule *src, IModule *dst) {
  for (ITable *src_tab : src->tables_) {
    ITable *new_tab = TableCopier::CopyTable(src_tab, dst);
    dst->tables_.push_back(new_tab);
  }
  vector<IModule *> child_mods = DesignUtil::GetChildModules(src);
  for (IModule *mod : child_mods) {
    IDesign *new_design = new_root_mod_->GetDesign();
    IModule *new_mod = new IModule(new_design, mod->GetName());
    TableCopier::CopyResourceParams(mod->GetParams(),
				    new_mod->GetParams());
    new_design->modules_.push_back(new_mod);
    new_mod->SetParentModule(dst);
    CopyRec(mod, new_mod);
    int id = UnusedId();
    new_mod->SetId(id);
    mod_id_in_new_design_[id] = new_mod;
    module_map_[mod] = new_mod;
  }
}

int ModuleCopier::UnusedId() {
  if (mod_id_in_new_design_.size() == 0) {
    return 1;
  }
  auto it = mod_id_in_new_design_.rbegin();
  int id = it->first + 1;
  if (id <= 0) {
    return 1;
  }
  return id;
}

}  // namespace iroha
