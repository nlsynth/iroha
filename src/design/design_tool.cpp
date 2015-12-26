#include "design/design_tool.h"

#include <set>

#include "design/resource_class.h"
#include "iroha/resource_params.h"

namespace iroha {

template<class T>
static void ValidateId(vector<T *> &v, set<int> &used_ids) {
  int last_id = 1;
  for (T *t : v) {
    if (t->GetId() < 0) {
      // Use first unused id.
      while (true) {
	if (used_ids.find(last_id) == used_ids.end()) {
	  break;
	}
	++last_id;
      }
      t->SetId(last_id);
      ++last_id;
    }
  }
}

DesignTool::DesignTool(IDesign *design) {
  if (design == nullptr) {
    design_ = new IDesign;
  } else {
    design_ = design;
  }
}

IDesign *DesignTool::GetDesign() {
  return design_;
}

void DesignTool::ValidateIds(ITable *table) {
  if (table == nullptr) {
    for (auto *mod : design_->modules_) {
      ValidateTableId(mod);
      for (auto *tab : mod->tables_) {
	ValidateStateId(tab);
	ValidateInsnId(tab);
	ValidateRegisterId(tab);
	ValidateResourceId(tab);
      }
    }
  } else {
    ValidateStateId(table);
    ValidateInsnId(table);
    ValidateRegisterId(table);
    ValidateResourceId(table);
  }
}

void DesignTool::ValidateTableId(IModule *mod) {
  set<int> used_ids;
  for (auto *tab : mod->tables_) {
    used_ids.insert(tab->GetId());
  }
  ValidateId(mod->tables_, used_ids);
}

void DesignTool::ValidateInsnId(ITable *table) {
  set<int> used_ids;
  vector<IInsn *> insns;
  for (auto *st : table->states_) {
    for (auto *insn : st->insns_) {
      used_ids.insert(insn->GetId());
      insns.push_back(insn);
    }
  }
  ValidateId(insns, used_ids);
}

void DesignTool::ValidateStateId(ITable *table) {
  set<int> used_ids;
  for (auto *st : table->states_) {
    used_ids.insert(st->GetId());
  }
  ValidateId(table->states_, used_ids);
}

void DesignTool::ValidateResourceId(ITable *table) {
  set<int> used_ids;
  for (auto *res : table->resources_) {
    used_ids.insert(res->GetId());
  }
  ValidateId(table->resources_, used_ids);
}

void DesignTool::ValidateRegisterId(ITable *table) {
  set<int> used_ids;
  for (auto *reg : table->registers_) {
    used_ids.insert(reg->GetId());
  }
  ValidateId(table->registers_, used_ids);
}

IInsn *DesignTool::AddNextState(IState *cur, IState *next) {
  ITable *table = cur->GetTable();
  IResource *tr = FindResourceByName(table, resource::kTransition);
  IInsn *insn = FindInsnByResource(cur, tr);
  if (insn == nullptr) {
    insn = new IInsn(tr);
    cur->insns_.push_back(insn);
  }
  insn->target_states_.push_back(next);
  return insn;
}

IResource *DesignTool::GetResource(ITable *table, const string &class_name) {
  IResource *res = FindResourceByName(table, class_name);
  if (res) {
    return res;
  }
  IDesign *design = table->GetModule()->GetDesign();
  IResourceClass *rc = FindResourceClass(design, class_name);
  if (!rc) {
    return nullptr;
  }
  res = new IResource(table, rc);
  table->resources_.push_back(res);
  return res;
}

IResource *DesignTool::GetBinOpResource(ITable *table,
					const string &class_name,
					int width) {
  for (auto *res : table->resources_) {
    if (res->GetClass()->GetName() == class_name &&
	res->input_types_[0].GetWidth() == width) {
      return res;
    }
  }
  IDesign *design = table->GetModule()->GetDesign();
  IResourceClass *rc = FindResourceClass(design, class_name);
  IResource *res = new IResource(table, rc);
  table->resources_.push_back(res);

  IValueType t;
  t.SetWidth(width);
  res->input_types_.push_back(t);
  res->input_types_.push_back(t);

  if (resource::IsNumToBoolBinOp(*(res->GetClass()))) {
    t.SetWidth(0);
  }
  res->output_types_.push_back(t);
  return res;
}

IResource *DesignTool::CreateEmbedResource(ITable *table,
					   const string &mod_name,
					   const string &fn) {
  IDesign *design = table->GetModule()->GetDesign();
  IResourceClass *rc = FindResourceClass(design, resource::kEmbedded);
  IResource *res = new IResource(table, rc);
  table->resources_.push_back(res);
  ResourceParams *params = res->GetParams();
  params->SetEmbeddedModuleName(mod_name, fn);
  return res;
}


IRegister *DesignTool::AllocRegister(ITable *table, const string &name,
				     int width) {
  IRegister *reg = new IRegister(table, name);
  reg->value_type_.SetWidth(width);
  table->registers_.push_back(reg);
  return reg;
}

IRegister *DesignTool::AllocConstNum(ITable *table,
				     int width, uint64_t value) {
  IRegister *reg = new IRegister(table, "");
  IValue v;
  v.value_ = value;
  v.type_.SetWidth(width);

  reg->SetInitialValue(v);
  reg->SetConst(true);
  table->registers_.push_back(reg);
  return reg;
}

void DesignTool::SetRegisterInitialValue(uint64_t value,
					 IRegister *reg) {
  IValue v;
  v.type_ = reg->value_type_;
  v.value_ = value;
  reg->SetInitialValue(v);
}

IResource *DesignTool::FindResourceByName(ITable *table, const string &name) {
  for (auto *res : table->resources_) {
    if (res->GetClass()->GetName() == name) {
      return res;
    }
  }
  return nullptr;
}

IInsn *DesignTool::FindInsnByResource(IState *state, IResource *res) {
  for (auto *insn : state->insns_) {
    if (insn->GetResource() == res) {
      return insn;
    }
  }
  return nullptr;
}

IResourceClass *DesignTool::FindResourceClass(IDesign *design,
					      const string &class_name) {
  for (auto *rc : design->resource_classes_) {
    if (rc->GetName() == class_name) {
      return rc;
    }
  }
  return nullptr;
}

}  // namespace iroha
