#include "builder/tree_builder.h"

#include "builder/exp_builder.h"
#include "builder/reader.h"
#include "iroha/i_design.h"
#include "iroha/logging.h"

namespace iroha {
namespace builder {

TreeBuilder::TreeBuilder(IDesign *design, ExpBuilder *builder)
  : design_(design), builder_(builder) {
}

void TreeBuilder::AddCalleeTable(const string &mod_name, int table_id,
				 IResource *res) {
  callee_module_names_[res] = mod_name;
  table_ids_[res] = table_id;
}

void TreeBuilder::AddForeignReg(int table_id, int reg_id, IResource *res) {
  foreign_registers_[res] = make_pair(table_id, reg_id);
}

void TreeBuilder::AddParentModule(const string &name, IModule *mod) {
  parent_module_names_[mod] = name;
}

bool TreeBuilder::Resolve() {
  map<string, IModule *> module_names;
  for (IModule *mod : design_->modules_) {
    module_names[mod->GetName()] = mod;
  }
  for (auto p : callee_module_names_) {
    IModule *mod = module_names[p.second];
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
    res->SetCalleeTable(mod->tables_[0]);
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
  for (auto f : foreign_registers_) {
    int table_id = f.second.first;
    int register_id = f.second.second;
    IResource *res = f.first;
    IModule *mod = res->GetTable()->GetModule();
    IRegister *foreign_reg = FindForeignRegister(mod, table_id, register_id);
    res->SetForeignRegister(foreign_reg);
  }
  return true;
}

IRegister *TreeBuilder::FindForeignRegister(IModule *mod,
					    int table_id, int register_id) {
  for (ITable *tab : mod->tables_) {
    if (tab->GetId() == table_id) {
      for (IRegister *reg : tab->registers_) {
	if (reg->GetId() == register_id) {
	  return reg;
	}
      }
    }
  }
  return nullptr;
}

}  // namespace builder
}  // namespace iroha
