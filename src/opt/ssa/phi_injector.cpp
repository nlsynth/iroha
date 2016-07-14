#include "opt/ssa/phi_injector.h"

#include "design/design_tool.h"
#include "design/design_util.h"
#include "iroha/i_design.h"
#include "iroha/logging.h"
#include "iroha/resource_class.h"
#include "opt/bb_set.h"
#include "opt/data_flow.h"
#include "opt/dominator_tree.h"
#include "opt/opt_util.h"

namespace iroha {
namespace opt {
namespace ssa {

PhiInjector::PhiInjector(ITable *table, DebugAnnotation *annotation)
  : table_(table), annotation_(annotation), phi_(nullptr) {
}

PhiInjector::~PhiInjector() {
}

void PhiInjector::Perform() {
  phi_ = DesignTool::GetOneResource(table_, resource::kPhi);
  tr_ = DesignUtil::FindTransitionResource(table_);
  bset_.reset(BBSet::Create(table_, annotation_));
  data_flow_.reset(DataFlow::Create(bset_.get(), annotation_));
  dom_tree_.reset(DominatorTree::Create(bset_.get(), annotation_));

  // Insert PHIs.
  CollectSingularRegister();
  for (IRegister *reg : table_->registers_) {
    if (reg->IsNormal()) {
      reg_phis_map_[reg] = new PerRegister;
    }
  }
  CollectOriginalDefs();
  PropagatePHIs();
}

void PhiInjector::CollectSingularRegister() {
  map<IRegister *, set<IInsn *> > define_insns;
  for (BB *bb : bset_->bbs_) {
    vector<RegDef *> reaches;
    data_flow_->GetReachDefs(bb, &reaches);
    map<IRegister *, set<IInsn *> > local_define_insns;
    // fill initial value by reaching assignments.
    for (RegDef *reg_def : reaches) {
      local_define_insns[reg_def->reg].insert(reg_def->insn);
    }
    for (IState *st : bb->states_) {
      for (auto *insn : st->insns_) {
	for (auto *ireg : insn->inputs_) {
	  set<IInsn *> &s = local_define_insns[ireg];
	  for (auto *local_insn : s) {
	    define_insns[ireg].insert(local_insn);
	  }
	}
      }
      // update write insns.
      for (IInsn *insn : st->insns_) {
	for (IRegister *oreg : insn->outputs_) {
	  // overwrite by this 1 assignment insn.
	  set<IInsn *> s;
	  s.insert(insn);
	  local_define_insns[oreg] = s;
	}
      }
    }
  }
  for (auto p : define_insns) {
    set<IInsn *> &s = p.second;
    IRegister *reg = p.first;
    if (s.size() == 1 && reg->IsNormal()) {
      singular_regs_.insert(reg);
    }
  }
}

void PhiInjector::CollectOriginalDefs() {
  for (BB *bb : bset_->bbs_) {
    vector<RegDef *> reach_defs;
    data_flow_->GetReachDefs(bb, &reach_defs);
    for (RegDef *reg_def : reach_defs) {
      if (singular_regs_.find(reg_def->reg) != singular_regs_.end()) {
	continue;
      }
      PerRegister *pr = GetPerRegister(reg_def->reg);
      pr->original_defs_.insert(reg_def);
    }
  }
}

PhiInjector::PerRegister *PhiInjector::GetPerRegister(IRegister *reg) {
  return reg_phis_map_[reg];
}

void PhiInjector::PropagatePHIs() {
  for (auto it : reg_phis_map_) {
    PerRegister *pr = it.second;
    // Setp 1. Generate PHIs from original assigns.
    for (RegDef *reg_def : pr->original_defs_) {
      PropagatePHIforBB(pr, reg_def->bb);
    }
    // Step 2. Propagete PHIs.
    int num_phis;
    do {
      num_phis = pr->phi_bbs_.size();
      for (BB *bb : pr->phi_bbs_) {
	PropagatePHIforBB(pr, bb);
      }
    } while (num_phis != pr->phi_bbs_.size());
  }
}

void PhiInjector::PropagatePHIforBB(PhiInjector::PerRegister *pr,
				     BB *bb) {
  vector<BB *> frontier;
  dom_tree_->GetFrontier(bb, &frontier);
  for (BB *bb : frontier) {
    pr->phi_bbs_.insert(bb);
  }
}

void PhiInjector::CommitPHIInsn() {
  // Allocate states for new PHI insns.
  set<BB *> bbs;
  for (auto it : reg_phis_map_) {
    PerRegister *pr = it.second;
    for (BB *bb : pr->phi_bbs_) {
      bbs.insert(bb);
    }
  }
  for (BB *bb : bbs) {
    PrependState(bb);
  }
  // Allocate insns.
  for (auto it : reg_phis_map_) {
    PerRegister *pr = it.second;
    for (BB *bb : pr->phi_bbs_) {
      IInsn *insn = new IInsn(phi_);
      insn->outputs_.push_back(it.first);
      bb->states_[0]->insns_.push_back(insn);
    }
  }
}

void PhiInjector::PrependState(BB *bb) {
  IState *head_st = bb->states_[0];
  ITable *tab = head_st->GetTable();
  auto it = tab->states_.begin();
  for (; it != tab->states_.end(); ++it) {
    if (*it == head_st) {
      break;
    }
  }
  CHECK(it != tab->states_.end());
  // vector.insert inserts before the iterator.
  ++it;
  IState *second_st = new IState(tab);
  tab->states_.insert(it, second_st);
  // Updates transitions.
  IInsn *head_tr_insn = DesignUtil::GetTransitionInsn(head_st);
  IInsn *second_tr_insn = DesignUtil::GetTransitionInsn(second_st);
  if (bb->states_.size() == 1) {
    second_tr_insn->target_states_ = head_tr_insn->target_states_;
    head_tr_insn->target_states_.clear();
    DesignTool::AddNextState(head_st, second_st);
  } else {
    // head=states_[0]
    // second.
    // states_[1]
    for (auto jt = head_tr_insn->target_states_.begin();
	 jt != head_tr_insn->target_states_.end(); ++jt) {
      if ((*jt) == bb->states_[1]) {
	*jt = second_st;
      }
    }
    DesignTool::AddNextState(second_st, bb->states_[1]);
  }
  // Move insns.
  bool again = true;
  while (again) {
    again = false;
    for (IInsn *insn : head_st->insns_) {
      if (insn->GetResource() != tr_) {
	again = true;
	DesignTool::MoveInsn(insn, head_st, second_st);
	break;
      }
    }
  }
}

}  // namespace ssa
}  // namespace opt
}  // namespace iroha
