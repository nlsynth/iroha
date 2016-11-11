#include "design/design_tool.h"

#include <set>

#include "design/design_util.h"
#include "design/importer.h"
#include "design/validator.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"

namespace iroha {

void DesignTool::Validate(IDesign *design) {
  Validator::Validate(design);
}

void DesignTool::ResolveImport(IDesign *design) {
  Importer::Import(design);
}

IInsn *DesignTool::AddNextState(IState *cur, IState *next) {
  IInsn *insn = DesignUtil::GetTransitionInsn(cur);
  insn->target_states_.push_back(next);
  return insn;
}

IState *DesignTool::InsertNextState(IState *st) {
  ITable *table = st->GetTable();
  auto it = table->states_.begin();
  for (IState *cur_st : table->states_) {
    if (cur_st == st) {
      break;
    }
    ++it;
  }
  if (it == table->states_.end()) {
    return nullptr;
  }
  IState *new_st = new IState(table);
  IInsn *st_tr = DesignUtil::GetTransitionInsn(st);
  IInsn *new_st_tr = DesignUtil::GetTransitionInsn(new_st);
  new_st_tr->target_states_ = st_tr->target_states_;
  st_tr->target_states_.clear();
  AddNextState(st, new_st);
  // inserts before |it|.
  ++it;
  table->states_.insert(it, new_st);
  return new_st;
}

void DesignTool::EraseInsn(IState *st, IInsn *insn) {
  for (auto it = st->insns_.begin(); it != st->insns_.end(); ++it) {
    if (*it == insn) {
      st->insns_.erase(it);
      return;
    }
  }
}

IResource *DesignTool::GetOneResource(ITable *table, const string &class_name) {
  IResource *res = DesignUtil::FindOneResourceByClassName(table, class_name);
  if (res) {
    return res;
  }
  IDesign *design = table->GetModule()->GetDesign();
  IResourceClass *rc = DesignUtil::FindResourceClass(design, class_name);
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
  IResourceClass *rc = DesignUtil::FindResourceClass(design, class_name);
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
  IResource *res = DesignUtil::CreateResource(table, resource::kArray);
  IValueType data_type;
  data_type.SetWidth(data_width);
  IArray *array = new IArray(res, address_width, data_type,
			     is_external, is_ram);
  res->SetArray(array);
  return res;
}

IResource *DesignTool::CreateShifterResource(ITable *table) {
  return DesignUtil::CreateResource(table, resource::kShift);
}

IResource *DesignTool::CreateEmbedResource(ITable *table,
					   const string &mod_name,
					   const string &fn) {
  IResource *res = DesignUtil::CreateResource(table, resource::kEmbedded);
  res->GetParams()->SetEmbeddedModuleName(mod_name, fn);
  return res;
}

IResource *DesignTool::CreateSubModuleTaskCallResource(ITable *table,
						       ITable *callee) {
  IResource *res = DesignUtil::CreateResource(table,
					      resource::kSubModuleTaskCall);
  res->SetCalleeTable(callee);
  return res;
}

IResource *DesignTool::CreateTaskResource(ITable *table) {
  return DesignUtil::CreateResource(table, resource::kSubModuleTask);
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
  IResource *shifter =
    DesignUtil::FindOneResourceByClassName(table, resource::kShift);
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

void DesignTool::DeleteInsn(IState *st, IInsn *insn) {
  auto it = st->insns_.begin();
  for (IInsn *cur : st->insns_) {
    if (cur == insn) {
      break;
    }
    ++it;
  }
  if (it == st->insns_.end()) {
    return;
  }
  st->insns_.erase(it);
}

void DesignTool::MoveInsn(IInsn *insn, IState *src_st, IState *dst_st) {
  dst_st->insns_.push_back(insn);
  int nth = 0;
  for (IInsn *cur_insn : src_st->insns_) {
    if (cur_insn == insn) {
      auto it = src_st->insns_.begin() + nth;
      src_st->insns_.erase(it);
      break;
    }
    ++nth;
  }
}

}  // namespace iroha
