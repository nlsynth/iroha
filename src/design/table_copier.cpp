#include "design/table_copier.h"

#include "iroha/resource_class.h"
#include "iroha/resource_params.h"

namespace iroha {

TableCopier::TableCopier(ITable *src, IModule *new_parent_mod)
  : mod_(new_parent_mod), src_tab_(src), new_tab_(new ITable(mod_)) {
}
  
ITable *TableCopier::CopyTable(ITable *src, IModule *new_parent_mod) {
  unique_ptr<TableCopier> copier(new TableCopier(src, new_parent_mod));
  return copier->Copy();
}

ITable *TableCopier::Copy() {
  CopyResource();
  CopyState();
  CopyRegister();
  CopyInsnAll();
  IState *src_initial_st = src_tab_->GetInitialState();
  if (src_initial_st != nullptr) {
    new_tab_->SetInitialState(state_map_[src_initial_st]);
  }
  new_tab_->SetId(src_tab_->GetId());
  return new_tab_;
}

void TableCopier::CopyResource() {
  BuildResourceClassMapping();

  IResource *new_tr = new_tab_->resources_[0];
  for (IResource *src_res : src_tab_->resources_) {
    auto *src_rc = src_res->GetClass();
    if (resource::IsTransition(*src_rc)) {
      resource_map_[src_res] = new_tr;
      new_tr->SetId(src_res->GetId());
    } else {
      IResource *new_res =
	new IResource(new_tab_, resource_class_map_[src_rc]);
      new_res->SetId(src_res->GetId());
      new_res->input_types_ = src_res->input_types_;
      new_res->output_types_ = src_res->output_types_;
      IArray *src_array = src_res->GetArray();
      if (src_array != nullptr) {
	new_res->SetArray(CopyArray(src_array, new_res));
      }
      CopyResourceParams(src_res->GetParams(), new_res->GetParams());
      resource_map_[src_res] = new_res;
      new_tab_->resources_.push_back(new_res);
    }
  }
}

void TableCopier::BuildResourceClassMapping() {
  map<string, IResourceClass *> src_rcs;
  IDesign *src_design = src_tab_->GetModule()->GetDesign();
  for (auto *rc : src_design->resource_classes_) {
    src_rcs[rc->GetName()] = rc;
  }
  IDesign *dst_design = new_tab_->GetModule()->GetDesign();
  for (auto *rc : dst_design->resource_classes_) {
    resource_class_map_[src_rcs[rc->GetName()]] = rc;
  }
}

void TableCopier::CopyState() {
  for (auto *src_st : src_tab_->states_) {
    IState *new_st = new IState(new_tab_);
    new_st->SetId(src_st->GetId());
    new_tab_->states_.push_back(new_st);
    state_map_[src_st] = new_st;
  }
}

void TableCopier::CopyRegister() {
  for (auto *src_reg : src_tab_->registers_) {
    IRegister *new_reg = new IRegister(new_tab_, src_reg->GetName());
    new_reg->SetId(src_reg->GetId());
    if (src_reg->HasInitialValue()) {
      IValue v = src_reg->GetInitialValue();
      new_reg->SetInitialValue(v);
    }
    new_reg->SetConst(src_reg->IsConst());
    new_reg->SetStateLocal(src_reg->IsStateLocal());
    new_reg->value_type_ = src_reg->value_type_;
    new_tab_->registers_.push_back(new_reg);
    reg_map_[src_reg] = new_reg;
  }
}

void TableCopier::CopyInsnAll() {
  for (auto *src_st : src_tab_->states_) {
    IState *new_st = state_map_[src_st];
    for (IInsn *src_insn : src_st->insns_) {
      IInsn *new_insn = CopyInsn(src_insn);
      new_st->insns_.push_back(new_insn);
    }
  }
}

IInsn *TableCopier::CopyInsn(IInsn *src_insn) {
  IInsn *new_insn = new IInsn(resource_map_[src_insn->GetResource()]);
  new_insn->SetId(src_insn->GetId());
  new_insn->SetOperand(src_insn->GetOperand());
  for (IRegister *r : src_insn->inputs_) {
    new_insn->inputs_.push_back(reg_map_[r]);
  }
  for (IRegister *r : src_insn->outputs_) {
    new_insn->outputs_.push_back(reg_map_[r]);
  }
  for (IState *s : src_insn->target_states_) {
    new_insn->target_states_.push_back(state_map_[s]);
  }
  return new_insn;
}

void TableCopier::CopyResourceParams(ResourceParams *src,
				     ResourceParams *dst) {
  vector<string> keys = src->GetParamKeys();
  for (string &key : keys) {
    vector<string> values = src->GetValues(key);
    dst->SetValues(key, values);
  }
}

IArray *TableCopier::CopyArray(IArray *src_array, IResource *new_res) {
  IArray *new_array = new IArray(new_res,
				 src_array->GetAddressWidth(),
				 src_array->GetDataType(),
				 src_array->IsExternal(),
				 src_array->IsRam());
  return new_array;
}

}  // namespace iroha
