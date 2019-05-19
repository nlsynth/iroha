// array_elimination phase changes an array into a set of registers if
// all indexes are constant.
#include "opt/array_elimination.h"

#include "design/design_tool.h"
#include "iroha/i_design.h"
#include "iroha/resource_class.h"

namespace iroha {
namespace opt {

ArrayElimination::~ArrayElimination() {
}

Phase *ArrayElimination::Create() {
  return new ArrayElimination();
}

bool ArrayElimination::ApplyForTable(const string &key, ITable *table) {
  set<IResource *> mems;
  // Collects internal arrays.
  for (IResource *res : table->resources_) {
    auto *klass = res->GetClass();
    if (resource::IsArray(*klass)) {
      IArray *array = res->GetArray();
      if (array->IsExternal()) {
	continue;
      }
      mems.insert(res);
    }
  }
  // Excludes arrays with non const index accessors.
  for (IState *st : table->states_) {
    for (IInsn *insn : st->insns_) {
      IResource *res = insn->GetResource();
      if (mems.find(res) == mems.end()) {
	continue;
      }
      IRegister *index = insn->inputs_[0];
      if (!index->IsConst()) {
	mems.erase(res);
      }
    }
  }
  // Injects assign insns.
  set<IInsn *> orig_insns;
  IResource *assign = DesignTool::GetOneResource(table, resource::kSet);
  for (IState *st : table->states_) {
    vector<IInsn *> new_insns;
    for (IInsn *insn : st->insns_) {
      IResource *res = insn->GetResource();
      if (mems.find(res) == mems.end()) {
	continue;
      }
      orig_insns.insert(insn);
      IRegister *index = insn->inputs_[0];
      IRegister *reg = GetRegister(res, index);
      IInsn *assign_insn = new IInsn(assign);
      if (insn->inputs_.size() == 1) {
	// read
	assign_insn->inputs_.push_back(reg);
	assign_insn->outputs_.push_back(insn->outputs_[0]);
      } else {
	// write
	assign_insn->inputs_.push_back(insn->inputs_[1]);
	assign_insn->outputs_.push_back(reg);
      }
      new_insns.push_back(assign_insn);
    }
    for (IInsn *insn : new_insns) {
      st->insns_.push_back(insn);
    }
  }
  // Removes original insns.
  for (IState *st : table->states_) {
    vector<IInsn *> insns;
    for (IInsn *insn : st->insns_) {
      if (orig_insns.find(insn) == orig_insns.end()) {
	insns.push_back(insn);
      }
    }
    st->insns_ = insns;
  }
  return true;
}

IRegister *ArrayElimination::GetRegister(IResource *array_res,
					 IRegister *index_reg) {

  int idx = index_reg->GetInitialValue().GetValue0();
  auto key = make_tuple(array_res, idx);
  auto it = fixed_regs_.find(key);
  if (it != fixed_regs_.end()) {
    return it->second;
  }
  string name = "a_" + Util::Itoa(array_res->GetId()) + "_" + Util::Itoa(idx);
  IRegister *reg = new IRegister(array_res->GetTable(), name);
  IArray *array = array_res->GetArray();
  reg->value_type_ = array->GetDataType();
  IArrayImage *img = array->GetArrayImage();
  if (img != nullptr) {
    Numeric n;
    n.SetValue0(img->values_[idx]);
    reg->SetInitialValue(n);
  }
  fixed_regs_[key] = reg;
  ITable *tab = array_res->GetTable();
  tab->registers_.push_back(reg);
  return reg;
}

}  // namespace opt
}  // namespace iroha
