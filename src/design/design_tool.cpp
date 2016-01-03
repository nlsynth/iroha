#include "design/design_tool.h"

#include <set>

#include "design/resource_class.h"
#include "design/validator.h"
#include "iroha/resource_params.h"

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

void DesignTool::Validate(ITable *table) {
  Validator::Validate(design_, table);
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

IResource *DesignTool::CreateArrayResource(ITable *table,
					   int address_width,
					   int data_width,
					   bool is_external,
					   bool is_ram) {
  IDesign *design = table->GetModule()->GetDesign();
  IResourceClass *rc = FindResourceClass(design, resource::kArray);
  if (rc == nullptr) {
    return nullptr;
  }
  IResource *res = new IResource(table, rc);
  IValueType data_type;
  data_type.SetWidth(data_width);
  IArray *array = new IArray(address_width, data_type,
			     is_external, is_ram);
  res->SetArray(array);
  table->resources_.push_back(res);
  return res;
}

IResource *DesignTool::CreateEmbedResource(ITable *table,
					   const string &mod_name,
					   const string &fn) {
  IDesign *design = table->GetModule()->GetDesign();
  IResourceClass *rc = FindResourceClass(design, resource::kEmbedded);
  if (rc == nullptr) {
    return nullptr;
  }
  IResource *res = new IResource(table, rc);
  table->resources_.push_back(res);
  ResourceParams *params = res->GetParams();
  params->SetEmbeddedModuleName(mod_name, fn);
  return res;
}

IResource *DesignTool::CreateSubModuleTaskCallResource(ITable *table,
						       IModule *mod) {
  IDesign *design = mod->GetDesign();
  IResourceClass *rc = FindResourceClass(design, resource::kSubModuleTaskCall);
  if (rc == nullptr) {
    return nullptr;
  }
  IResource *res = new IResource(table, rc);
  res->SetModule(mod);
  table->resources_.push_back(res);
  return res;
}

IResource *DesignTool::CreateTaskResource(ITable *table) {
  IDesign *design = table->GetModule()->GetDesign();
  IResourceClass *rc = FindResourceClass(design, resource::kSubModuleTask);
  if (rc == nullptr) {
    return nullptr;
  }
  IResource *res = new IResource(table, rc);
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
