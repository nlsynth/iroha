#include "design/util.h"

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

IResource *DesignUtil::FindResourceByClassName(ITable *table,
					       const string &name) {
  for (auto *res : table->resources_) {
    if (res->GetClass()->GetName() == name) {
      return res;
    }
  }
  return nullptr;
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

IInsn *DesignUtil::GetTransitionInsn(IState *st) {
  ITable *table = st->GetTable();
  IResource *tr = DesignUtil::FindResourceByClassName(table,
						      resource::kTransition);
  IInsn *insn = DesignUtil::FindInsnByResource(st, tr);
  if (insn == nullptr) {
    insn = new IInsn(tr);
    st->insns_.push_back(insn);
  }
  return insn;
}

}  // namespace iroha
