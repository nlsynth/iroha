#include "builder/tree_builder.h"

#include "builder/exp_builder.h"
#include "builder/reader.h"
#include "iroha/i_design.h"
#include "iroha/logging.h"
#include "iroha/module_import.h"

namespace iroha {
namespace builder {

TreeBuilder::TreeBuilder(IDesign *design, ExpBuilder *builder)
  : design_(design), builder_(builder) {
}

void TreeBuilder::AddCalleeTable(int mod_id, int table_id,
				 IResource *res) {
  callee_module_ids_[res] = mod_id;
  table_ids_[res] = table_id;
}

void TreeBuilder::AddParentModule(int parent_mod_id, IModule *mod) {
  parent_module_ids_[mod] = parent_mod_id;
}

void TreeBuilder::AddParentResource(int module_id, int table_id, int res_id,
				    IResource *res) {
  ParentResource pr;
  pr.mod_id = module_id;
  pr.tab_id = table_id;
  pr.res_id = res_id;
  pr.reader = res;
  parent_resources_.push_back(pr);
}

void TreeBuilder::AddArrayImage(IArray *array, int imageid) {
  ArrayImage im;
  im.array = array;
  im.imageid = imageid;
  array_images_.push_back(im);
}

void TreeBuilder::AddModuleImportTap(int module_id, int table_id, int res_id,
				     ModuleImportTap *tap) {
  Tap t;
  t.tap = tap;
  t.mod_id = module_id;
  t.tab_id = table_id;
  t.res_id = res_id;
  taps_.push_back(t);
}

bool TreeBuilder::Resolve() {
  map<int, IModule *> module_ids;
  for (IModule *mod : design_->modules_) {
    module_ids[mod->GetId()] = mod;
  }
  for (auto p : callee_module_ids_) {
    IModule *mod = module_ids[p.second];
    if (mod == nullptr) {
      builder_->SetError() << "unknown module: " << p.second;
      return false;
    }
    IResource *res = p.first;
    int table_id = table_ids_[res];
    ITable *callee_tab = nullptr;
    for (ITable *t : mod->tables_) {
      if (t->GetId() == table_id) {
	callee_tab = t;
      }
    }
    CHECK(callee_tab != nullptr);
    res->SetCalleeTable(callee_tab);
  }
  for (auto p : parent_module_ids_) {
    IModule *mod = module_ids[p.second];
    if (mod == nullptr) {
      builder_->SetError() << "unknown module: " << p.second;
      return false;
    }
    IModule *cmod = p.first;
    cmod->SetParentModule(mod);
  }
  for (auto &pr : parent_resources_) {
    IModule *mod = module_ids[pr.mod_id];
    if (mod == nullptr) {
      builder_->SetError() << "no shared reg reader module id: " << pr.mod_id;
      return false;
    }
    IResource *res = FindResource(mod, pr.tab_id, pr.res_id);
    pr.reader->SetParentResource(res);
  }
  map<int, IArrayImage *> array_ids;
  for (auto *im : design_->array_images_) {
    array_ids[im->GetId()] = im;
  }
  for (auto &im : array_images_) {
    auto it = array_ids.find(im.imageid);
    if (it == array_ids.end()) {
      builder_->SetError() << "failed to find array image: " << im.imageid;
    }
    im.array->SetArrayImage(it->second);
  }
  for (auto &t : taps_) {
    IModule *mod = module_ids[t.mod_id];
    if (mod == nullptr) {
      builder_->SetError() << "no tap to connect module id: " << t.mod_id;
      return false;
    }
    t.tap->resource = FindResource(mod, t.tab_id, t.res_id);
  }
  return true;
}

IResource *TreeBuilder::FindResource(IModule *mod,
				     int table_id, int resource_id) {
  for (ITable *tab : mod->tables_) {
    if (tab->GetId() == table_id) {
      for (IResource *res : tab->resources_) {
	if (res->GetId() == resource_id) {
	  return res;
	}
      }
    }
  }
  return nullptr;
}

}  // namespace builder
}  // namespace iroha
