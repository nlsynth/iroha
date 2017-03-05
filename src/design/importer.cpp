#include "design/importer.h"

#include "design/design_util.h"
#include "design/module_copier.h"
#include "design/table_copier.h"
#include "iroha/i_design.h"
#include "iroha/iroha.h"
#include "iroha/logging.h"
#include "iroha/module_import.h"
#include "iroha/resource_class.h"

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
  ProcessTapAll();
  ClearModuleImport();
}

void Importer::TraverseModule(IModule *mod) {
  ModuleImport *mi = mod->GetModuleImport();
  if (mi != nullptr) {
    ProcessImport(mod);
    // Don`t process sub modules just imported.
    return;
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

void Importer::ProcessTapAll() {
  for (IModule *mod : design_->modules_) {
    if (mod->GetModuleImport() != nullptr) {
      ProcessTap(mod);
    }
  }
  for (auto &p : tag_to_resources_) {
    auto &v = p.second;
    CHECK(v.size() == 2);
    if (resource::IsSharedReg(*(v[0]->GetClass()))) {
      ConnectResources(v[0], v[1]);
    } else {
      ConnectResources(v[1], v[0]);
    }
  }
}

void Importer::ConnectResources(IResource *w, IResource *r) {
  r->SetParentResource(w);
}

void Importer::ProcessTap(IModule *mod) {
  map<string, IResource * > name_to_resource;
  for (ITable *tab : mod->tables_) {
    for (IResource *res : tab->resources_) {
      auto *rc = res->GetClass();
      auto *params = res->GetParams();
      if (resource::IsExtInput(*rc)) {
	string input;
	params->GetExtInputPort(&input, nullptr);
	name_to_resource[input] = res;
      }
      if (resource::IsExtOutput(*rc)) {
	string output;
	params->GetExtOutputPort(&output, nullptr);
	name_to_resource[output] = res;
      }
    }
  }
  ModuleImport *mi = mod->GetModuleImport();
  for (auto *tap : mi->taps_) {
    IResource *orig_res = name_to_resource[tap->source];
    IResource *res = RemapResource(*tap, orig_res);
    CHECK(res) << tap->source;
    if (!tap->tag.empty()) {
      tag_to_resources_[tap->tag].push_back(res);
    }
    if (tap->resource) {
      res->SetParentResource(tap->resource);
    }
  }
}

void Importer::ClearModuleImport() {
  for (IModule *mod : design_->modules_) {
    if (mod->GetModuleImport() != nullptr) {
      mod->SetModuleImport(nullptr);
    }
  }
}

IResource *Importer::RemapResource(const ModuleImportTap &tap,
				   IResource *src_res) {
  ITable *tab = src_res->GetTable();
  IResourceClass *rc = DesignUtil::FindResourceClass(design_,
						     tap.resource_class);
  IResource *res = new IResource(tab, rc);
  tab->resources_.push_back(res);
  for (IState *st : tab->states_) {
    vector<IInsn *> insns;
    for (IInsn *insn : st->insns_) {
      if (insn->GetResource() == src_res) {
	IInsn *new_insn = new IInsn(res);
	TableCopier::CopyInsnParams(insn, new_insn);
	insns.push_back(new_insn);
      } else {
	insns.push_back(insn);
      }
    }
    st->insns_ = insns;
  }
  // Remove src_res.
  vector<IResource *> resources;
  for (IResource *r : tab->resources_) {
    if (r != src_res) {
      resources.push_back(r);
    }
  }
  tab->resources_ = resources;
  return res;
}

}  // namespace iroha
