#include "opt/ssa/phi_cleaner.h"

#include "design/design_tool.h"
#include "iroha/resource_class.h"
#include "opt/bb_set.h"
#include "opt/data_flow.h"

namespace iroha {
namespace opt {
namespace ssa {

PhiCleaner::PhiCleaner(ITable *table, DebugAnnotation *annotation)
  : table_(table), annotation_(annotation), nth_sel_(0) {
}

PhiCleaner::~PhiCleaner() {
}

void PhiCleaner::Perform() {
  phi_ = DesignTool::GetOneResource(table_, resource::kPhi);
  sel_ = DesignTool::GetOneResource(table_, resource::kSelect);
  assign_ = DesignTool::GetOneResource(table_, resource::kSet);
  bset_.reset(BBSet::Create(table_, annotation_));
  data_flow_.reset(DataFlow::Create(bset_.get(), annotation_));

  for (RegDef *reg_def : data_flow_->all_defs_) {
    reg_def_map_[reg_def->insn].insert(reg_def);
  }

  for (BB *bb : bset_->bbs_) {
    ProcessBB(bb);
  }
}

void PhiCleaner::ProcessBB(BB *bb) {
  vector<RegDef *> reaches;
  data_flow_->GetReachDefs(bb, &reaches);
  map<IRegister *, RegDef *> last_defs;
  for (RegDef *reg_def : reaches) {
    last_defs[reg_def->reg] = reg_def;
  }
  for (IState *st : bb->states_) {
    // Copy this list to allow ProcessInsn() to modify the list.
    vector<IInsn *> copied_insns = st->insns_;
    for (IInsn *insn : copied_insns) {
      ProcessInsn(&last_defs, st, insn);
    }
  }
}

void PhiCleaner::ProcessInsn(map<IRegister *, RegDef *> *last_defs,
			     IState *st, IInsn *insn) {
  for (IRegister *reg : insn->outputs_) {
    set<RegDef *> &defs = reg_def_map_[insn];
    RegDef *reg_def = nullptr;
    for (RegDef *d : defs) {
      if (d->reg == reg) {
	reg_def = d;
      }
    }
    (*last_defs)[reg] = reg_def;
  }
  if (insn->GetResource() != phi_) {
    return;
  }
  if (insn->inputs_.size() == 1) {
    IInsn *assign_insn = new IInsn(assign_);
    assign_insn->inputs_ = insn->inputs_;
    assign_insn->outputs_ = insn->outputs_;
    st->insns_.push_back(assign_insn);
  } else {
    EmitSelector(last_defs, st, insn);
  }
  // Removes the phi.
  DesignTool::EraseInsn(st, insn);
}

void PhiCleaner::EmitSelector(map<IRegister *, RegDef *> *last_defs,
			      IState *st, IInsn *phi_insn) {
  // Condition reg for each BB.
  IRegister *cond_reg = new IRegister(table_, "cond_" + Util::Itoa(nth_sel_));
  table_->registers_.push_back(cond_reg);
  cond_reg->value_type_.SetWidth(0);
  int nth = 0;
  for (IRegister *reg : phi_insn->inputs_) {
    IInsn *assign_insn = new IInsn(assign_);
    assign_insn->outputs_.push_back(cond_reg);
    IRegister *reg_val = DesignTool::AllocConstNum(table_, 0, nth);
    assign_insn->inputs_.push_back(reg_val);
    RegDef *reg_def = (*last_defs)[reg];
    reg_def->st->insns_.push_back(assign_insn);
    ++nth;
  }
  // Emit a selector.
  IInsn *sel_insn = new IInsn(sel_);
  sel_insn->inputs_.push_back(cond_reg);
  for (IRegister *reg : phi_insn->inputs_) {
    sel_insn->inputs_.push_back(reg);
  }
  IRegister *output_reg = *(phi_insn->outputs_.begin());
  sel_insn->outputs_.push_back(output_reg);
  st->insns_.push_back(sel_insn);
}

}  // namespace ssa
}  // namespace opt
}  // namespace iroha
