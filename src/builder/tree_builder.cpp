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

void TreeBuilder::AddCalleeTable(int mod_id, int table_id,
				 IResource *res) {
  callee_module_ids_[res] = mod_id;
  table_ids_[res] = table_id;
}

void TreeBuilder::AddForeignReg(int module_id, int table_id, int reg_id,
				IResource *res) {
  ForeignRegister reg;
  reg.table_id = table_id;
  reg.reg_id = reg_id;
  reg.mod_id = module_id;
  foreign_registers_[res] = reg;
}

void TreeBuilder::AddParentModule(int parent_mod_id, IModule *mod) {
  parent_module_ids_[mod] = parent_mod_id;
}

void TreeBuilder::AddChannelReaderWriter(IChannel *ch, bool is_r,
					 int mod_id,
					 int tab_id, int res_id) {
  ChannelEndPoint ep;
  ep.ch = ch;
  ep.is_r = is_r;
  ep.mod_id = mod_id;
  ep.tab_id = tab_id;
  ep.res_id = res_id;
  channel_end_points_.push_back(ep);
}

void TreeBuilder::AddPortInput(int module_id, int table_id, int res_id,
			       IResource *res) {
  PortInput port;
  port.mod_id = module_id;
  port.tab_id = table_id;
  port.res_id = res_id;
  port.reader = res;
  port_inputs_.push_back(port);
}

void TreeBuilder::AddArrayImage(IArray *array, int imageid) {
  ArrayImage im;
  im.array = array;
  im.imageid = imageid;
  array_images_.push_back(im);
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
  for (auto f : foreign_registers_) {
    int mod_id = f.second.mod_id;
    int table_id = f.second.table_id;
    int register_id = f.second.reg_id;
    IResource *res = f.first;
    IModule *mod = module_ids[mod_id];
    IRegister *foreign_reg = FindForeignRegister(mod, table_id, register_id);
    res->SetForeignRegister(foreign_reg);
  }
  for (auto &ep : channel_end_points_) {
    IModule *mod = module_ids[ep.mod_id];
    if (mod == nullptr) {
      builder_->SetError() << "no endpoint module id: " << ep.mod_id;
      return false;
    }
    IResource *res = FindResource(mod, ep.tab_id, ep.res_id);
    if (ep.is_r) {
      ep.ch->SetReader(res);
    } else {
      ep.ch->SetWriter(res);
    }
  }
  for (auto &port : port_inputs_) {
    IModule *mod = module_ids[port.mod_id];
    if (mod == nullptr) {
      builder_->SetError() << "no port reader module id: " << port.mod_id;
      return false;
    }
    IResource *res = FindResource(mod, port.tab_id, port.res_id);
    port.reader->SetPortInput(res);
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
