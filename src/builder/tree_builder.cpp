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

void TreeBuilder::AddForeignReg(int table_id, int reg_id,
				const string &mod_name,
				IResource *res) {
  ForeignRegister reg;
  reg.table_id = table_id;
  reg.reg_id = reg_id;
  reg.mod = mod_name;
  foreign_registers_[res] = reg;
}

void TreeBuilder::AddParentModule(const string &name, IModule *mod) {
  parent_module_names_[mod] = name;
}

void TreeBuilder::AddChannelReaderWriter(IChannel *ch, bool is_r,
					 const string &mod_name,
					 int tab_id, int res_id) {
  ChannelEndPoint ep;
  ep.ch = ch;
  ep.is_r = is_r;
  ep.mod_name = mod_name;
  ep.tab_id = tab_id;
  ep.res_id = res_id;
  channel_end_points_.push_back(ep);
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
    res->SetCalleeTable(callee_tab);
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
    int table_id = f.second.table_id;
    int register_id = f.second.reg_id;
    string &mod_name = f.second.mod;
    IResource *res = f.first;
    IModule *mod;
    if (mod_name.empty()) {
      mod = res->GetTable()->GetModule();
    } else {
      mod = module_names[mod_name];
    }
    IRegister *foreign_reg = FindForeignRegister(mod, table_id, register_id);
    res->SetForeignRegister(foreign_reg);
  }
  for (auto &ep : channel_end_points_) {
    IModule *mod = module_names[ep.mod_name];
    IResource *res = FindChannelResource(mod, ep.tab_id, ep.res_id);
    if (ep.is_r) {
      ep.ch->SetReader(res);
    } else {
      ep.ch->SetWriter(res);
    }
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

IResource *TreeBuilder::FindChannelResource(IModule *mod,
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
