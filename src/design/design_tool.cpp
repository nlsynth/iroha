#include "design/design_tool.h"

#include <set>

#include "design/resource_class.h"

namespace iroha {

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

void DesignTool::ValidateStateId(ITable *table) {
  if (table == nullptr) {
    for (auto *mod : design_->modules_) {
      for (auto *tab : mod->tables_) {
	ValidateStateId(tab);
      }
    }
    return;
  }
  set<int> used_ids;
  for (auto *st : table->states_) {
    used_ids.insert(st->GetId());
  }
  int last_id = 1;
  for (auto *st : table->states_) {
    if (st->GetId() < 0) {
      // Use first unused id.
      while (true) {
	if (used_ids.find(last_id) == used_ids.end()) {
	  break;
	}
	++last_id;
      }
      st->SetId(last_id);
      ++last_id;
    }
  }
}

void DesignTool::SetNextState(IState *cur, IState *next) {
  ITable *table = cur->GetTable();
  IResource *tr = FindResourceByName(table, resource::kTransition);
  IInsn *insn = new IInsn(tr);
  insn->target_states_.push_back(next);
  cur->insns_.push_back(insn);
}

IResource *DesignTool::GetResource(ITable *table, const string &class_name) {
  IResource *res = FindResourceByName(table, class_name);
  if (res) {
    return res;
  }
  IDesign *design = table->GetModule()->GetDesign();
  IResourceClass *rc = FindResourceClass(design, class_name);
  res = new IResource(table, rc);
  table->resources_.push_back(res);
  return res;
}

IRegister *DesignTool::AllocRegister(ITable *table, const string &name,
				     int width) {
  IRegister *reg = new IRegister(table, name);
  reg->value_type_.SetWidth(width);
  table->registers_.push_back(reg);
  return reg;
}

IRegister *DesignTool::AllocConstNum(ITable *table, const string &name,
				     int width, uint64_t value) {
  IRegister *reg = new IRegister(table, name);
  IValue v;
  v.value_ = value;
  v.type_.SetIsEnum(false);
  v.type_.SetWidth(width);

  reg->SetInitialValue(v);
  reg->is_const_ = true;
  table->registers_.push_back(reg);
  return reg;
}

IResource *DesignTool::FindResourceByName(ITable *table, const string &name) {
  for (auto *res : table->resources_) {
    if (res->GetClass()->GetName() == name) {
      return res;
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
