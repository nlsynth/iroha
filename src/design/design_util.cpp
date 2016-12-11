#include "design/design_util.h"

#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "iroha/logging.h"

namespace iroha {

IModule *DesignUtil::GetRootModule(const IDesign *design) {
  IModule *root = nullptr;
  for (auto *mod : design->modules_) {
    if (mod->GetParentModule() == nullptr) {
      if (root == nullptr) {
	root = mod;
      } else {
	// Don't allow multiple roots.
	return nullptr;
      }
    }
  }
  return root;
}

vector<IModule *> DesignUtil::GetChildModules(const IModule *parent) {
  const IDesign *design = parent->GetDesign();
  vector<IModule *> v;
  for (auto *mod : design->modules_) {
    if (mod->GetParentModule() == parent) {
      v.push_back(mod);
    }
  }
  return v;
}

IResourceClass *DesignUtil::GetTransitionResourceClassFromDesign(IDesign *design) {
  for (auto *rc : design->resource_classes_) {
    if (rc->GetName() == resource::kTransition) {
      return rc;
    }
  }
  CHECK(false) << "Transition resource class is not installed?";
  return nullptr;
}

void DesignUtil::FindResourceByClassName(ITable *table,
					 const string &name,
					 vector<IResource *> *resources) {
  for (auto *res : table->resources_) {
    if (res->GetClass()->GetName() == name) {
      resources->push_back(res);
    }
  }
}

IResource *DesignUtil::FindOneResourceByClassName(ITable *table, const string &name) {
  vector<IResource *> resources;
  FindResourceByClassName(table, name, &resources);
  if (resources.size() == 0) {
    return nullptr;
  }
  CHECK(resources.size() == 1) << "the table has " << resources.size() << " " << name;
  return resources[0];
}

IResource *DesignUtil::FindAssignResource(ITable *table) {
  return FindOneResourceByClassName(table, resource::kSet);
}

IResource *DesignUtil::FindTransitionResource(ITable *table) {
  return FindOneResourceByClassName(table, resource::kTransition);
}

IInsn *DesignUtil::FindInsnByResource(IState *state, IResource *res) {
  for (auto *insn : state->insns_) {
    if (insn->GetResource() == res) {
      return insn;
    }
  }
  return nullptr;
}

IResourceClass *DesignUtil::FindResourceClass(IDesign *design,
					      const string &class_name) {
  for (auto *rc : design->resource_classes_) {
    if (rc->GetName() == class_name) {
      return rc;
    }
  }
  return nullptr;
}

IResource *DesignUtil::CreateResource(ITable *table, const string &name) {
  IDesign *design = table->GetModule()->GetDesign();
  IResourceClass *rc = FindResourceClass(design, name);
  if (rc == nullptr) {
    return nullptr;
  }
  IResource *res = new IResource(table, rc);
  table->resources_.push_back(res);
  return res;
}

IInsn *DesignUtil::FindTransitionInsn(IState *st) {
  ITable *table = st->GetTable();
  IResource *tr = DesignUtil::FindOneResourceByClassName(table,
							 resource::kTransition);
  return DesignUtil::FindInsnByResource(st, tr);
}

IInsn *DesignUtil::GetTransitionInsn(IState *st) {
  IInsn *insn = FindTransitionInsn(st);
  if (insn == nullptr) {
    ITable *table = st->GetTable();
    IResource *tr = DesignUtil::FindOneResourceByClassName(table,
							   resource::kTransition);
    insn = new IInsn(tr);
    st->insns_.push_back(insn);
  }
  return insn;
}

IInsn *DesignUtil::FindTaskEntryInsn(ITable *table) {
  return FindInitialInsnByClassName(table, resource::kTask);
}

IInsn *DesignUtil::FindInitialInsnByClassName(ITable *table, const string &name) {
  IResource *res = FindOneResourceByClassName(table, name);
  if (res != nullptr) {
    return DesignUtil::FindInsnByResource(table->GetInitialState(), res);
  }
  return nullptr;
}

IInsn *DesignUtil::FindDataFlowInInsn(ITable *table) {
  return FindInitialInsnByClassName(table, resource::kDataFlowIn);
}

bool DesignUtil::IsTerminalState(IState *st) {
  ITable *table = st->GetTable();
  IResource *tr = DesignUtil::FindTransitionResource(table);
  IInsn *insn = DesignUtil::FindInsnByResource(st, tr);
  if (insn == nullptr) {
    return true;
  }
  if (insn->target_states_.size() == 0) {
    return true;
  }
  return false;
}

bool DesignUtil::IsMultiCycleInsn(IInsn *insn) {
  IResource *res = insn->GetResource();
  IResourceClass *rc = res->GetClass();
  if (resource::IsTaskCall(*rc) ||
      resource::IsChannelRead(*rc) ||
      resource::IsChannelWrite(*rc) ||
      resource::IsEmbedded(*rc)) {
    return true;
  }
  return false;
}

int DesignUtil::NumMultiCycleInsn(IState *st) {
  int n = 0;
  for (IInsn *insn : st->insns_) {
    if (IsMultiCycleInsn(insn)) {
      ++n;
    }
  }
  return n;
}

IResource *DesignUtil::FindResourceById(ITable *tab,
					int res_id) {
  for (IResource *res : tab->resources_) {
    if (res->GetId() == res_id) {
      return res;
    }
  }
  return nullptr;
}

ITable *DesignUtil::FindTableById(IModule *mod, int tab_id) {
  for (ITable *tab : mod->tables_) {
    if (tab->GetId() == tab_id) {
      return tab;
    }
  }
  return nullptr;
}

IRegister *DesignUtil::FindRegisterById(ITable *tab, int reg_id) {
  for (IRegister *reg : tab->registers_) {
    if (reg->GetId() == reg_id) {
      return reg;
    }
  }
  return nullptr;
}

}  // namespace iroha
