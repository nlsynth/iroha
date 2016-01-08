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
  IResource *tr = FindResourceByClassName(table, resource::kTransition);
  IInsn *insn = FindInsnByResource(cur, tr);
  if (insn == nullptr) {
    insn = new IInsn(tr);
    cur->insns_.push_back(insn);
  }
  insn->target_states_.push_back(next);
  return insn;
}

IResource *DesignTool::GetResource(ITable *table, const string &class_name) {
  IResource *res = FindResourceByClassName(table, class_name);
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

  if (resource::IsNumToBoolExclusiveBinOp(*(res->GetClass()))) {
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
  IResource *res = CreateResource(table, resource::kArray);
  IValueType data_type;
  data_type.SetWidth(data_width);
  IArray *array = new IArray(address_width, data_type,
			     is_external, is_ram);
  res->SetArray(array);
  return res;
}

IResource *DesignTool::CreateShifterResource(ITable *table) {
  return CreateResource(table, resource::kShift);
}

IResource *DesignTool::CreateEmbedResource(ITable *table,
					   const string &mod_name,
					   const string &fn) {
  IResource *res = CreateResource(table, resource::kEmbedded);
  res->GetParams()->SetEmbeddedModuleName(mod_name, fn);
  return res;
}

IResource *DesignTool::CreateSubModuleTaskCallResource(ITable *table,
						       IModule *mod) {
  IResource *res = CreateResource(table, resource::kSubModuleTaskCall);
  res->SetModule(mod);
  return res;
}

IResource *DesignTool::CreateTaskResource(ITable *table) {
  return CreateResource(table, resource::kSubModuleTask);
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

IInsn *DesignTool::CreateShiftInsn(IRegister *reg, bool to_left, int amount) {
  ITable *table = reg->GetTable();
  IResource *shifter = FindResourceByClassName(table, resource::kShift);
  IInsn *insn = new IInsn(shifter);
  if (to_left) {
    insn->SetOperand("left");
  } else {
    insn->SetOperand("right");
  }
  insn->inputs_.push_back(reg);
  IRegister *a = AllocConstNum(table, 32, amount);
  insn->inputs_.push_back(a);
  return insn;
}

IResource *DesignTool::FindResourceByClassName(ITable *table,
					       const string &name) {
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

IResource *DesignTool::CreateResource(ITable *table, const string &name) {
  IDesign *design = table->GetModule()->GetDesign();
  IResourceClass *rc = FindResourceClass(design, name);
  if (rc == nullptr) {
    return nullptr;
  }
  IResource *res = new IResource(table, rc);
  table->resources_.push_back(res);
  return res;
}

}  // namespace iroha
