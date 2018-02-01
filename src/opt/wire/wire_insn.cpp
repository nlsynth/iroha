#include "opt/wire/wire_insn.h"

#include "iroha/logging.h"
#include "design/design_util.h"
#include "design/design_tool.h"
#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "opt/bb_set.h"
#include "opt/data_flow.h"
#include "opt/debug_annotation.h"

namespace iroha {
namespace opt {
namespace wire {

WireInsnPhase::~WireInsnPhase() {
}

Phase *WireInsnPhase::Create() {
  return new WireInsnPhase();
}

bool WireInsnPhase::ApplyForTable(const string &key, ITable *table) {
  WireInsn wire_insn(table, annotation_);
  return wire_insn.Perform();
}

WireInsn::WireInsn(ITable *table, DebugAnnotation *annotation)
  : table_(table), annotation_(annotation), bset_(nullptr),
    data_flow_(nullptr) {
}

WireInsn::~WireInsn() {
  delete bset_;
  delete data_flow_;
}

bool WireInsn::Perform() {
  assign_ = DesignTool::GetOneResource(table_, resource::kSet);
  transition_ = DesignUtil::FindTransitionResource(table_);
  bset_ = BBSet::Create(table_, annotation_);
  data_flow_ = DataFlow::Create(bset_, annotation_);
  if (annotation_ != nullptr) {
    annotation_->DumpIntermediateTable(table_);
  }
  CollectReachingRegisters();
  for (BB *bb : bset_->bbs_) {
    BuildDependency(bb);
  }

  for (BB *bb : bset_->bbs_) {
    ReplaceInsnOutputWithWireBB(bb);
    ScanBBToMoveInsn(bb);
  }
  AddWireToRegisterAssignments();
  for (BB *bb : bset_->bbs_) {
    MoveLastTransitionInsn(bb);
  }
  if (annotation_ != nullptr) {
    annotation_->DumpIntermediateTable(table_);
  }
  return true;
}

void WireInsn::CollectReachingRegisters() {
  CollectUsedRegsPerBB();
  // Collect defs used somewhere in this table.
  set<RegDef *> active_defs;
  for (BB *bb : bset_->bbs_) {
    vector<RegDef *> reach_defs;
    data_flow_->GetReachDefs(bb, &reach_defs);
    auto bb_regs = used_regs_[bb];
    for (RegDef *reg_def : reach_defs) {
      if (bb_regs.find(reg_def->reg) != bb_regs.end()) {
	active_defs.insert(reg_def);
      }
    }
  }

  for (RegDef *reg_def : active_defs) {
    PerInsn *pi = GetPerInsn(reg_def->insn);
    for (IRegister *oreg : reg_def->insn->outputs_) {
      if (oreg == reg_def->reg) {
	// TODO(yt76): should be reach && used.
	// now this just checks only the reachability.
	pi->output_reach_to_other_bb_.insert(oreg);
      }
    }
  }
}

void WireInsn::CollectUsedRegsPerBB() {
  for (IState *st : table_->states_) {
    BB *bb = bset_->state_to_bb_[st];
    for (IInsn *insn : st->insns_) {
      for (IRegister *ireg : insn->inputs_) {
	used_regs_[bb].insert(ireg);
      }
    }
  }
}

void WireInsn::BuildDependency(BB *bb) {
  map<IRegister *, IInsn *> last_def_insn;
  map<IRegister *, IInsn *> last_read_insn;
  int nth_state = 0;
  for (IState *st : bb->states_) {
    for (IInsn *insn : st->insns_) {
      PerInsn *pi = GetPerInsn(insn);
      pi->nth_state = nth_state;
      // WRITE -> READ dependency
      for (IRegister *reg : insn->inputs_) {
        BuildRWDependencyPair(insn, reg, last_def_insn);
      }
      // READ -> WRITE dependency
      for (IRegister *reg : insn->outputs_) {
        BuildRWDependencyPair(insn, reg, last_read_insn);
      }
      // Update last write
      for (IRegister *reg : insn->outputs_) {
	last_def_insn[reg] = insn;
      }
      // Update last read
      for (IRegister *reg : insn->inputs_) {
	last_read_insn[reg] = insn;
      }
    }
    ++nth_state;
  }
}

void WireInsn::BuildRWDependencyPair(IInsn *insn, IRegister *source_reg,
				     map<IRegister *, IInsn *> &dep_map) {
  IInsn *def_insn = dep_map[source_reg];
  if (!def_insn) {
    // not written/read in this block.
    return;
  }
  PerInsn *pi = GetPerInsn(insn);
  pi->depending_insn_[source_reg] = def_insn;
  // adds reverse mapping too.
  PerInsn *def_insn_pi = GetPerInsn(def_insn);
  def_insn_pi->using_insns_[source_reg].insert(insn);
}

void WireInsn::ReplaceInsnOutputWithWireBB(BB *bb) {
  for (IState *st : bb->states_) {
    for (IInsn *insn : st->insns_) {
      // TODO(yt76): Don't touch multi cycles insns like memory ops.
      if (insn->GetResource() == assign_) {
        PerInsn *pi = GetPerInsn(insn);
        pi->is_assign = true;
      } else {
        ReplaceInsnOutputWithWire(insn);
      }
    }
  }
}

void WireInsn::ReplaceInsnOutputWithWire(IInsn *insn) {
  for (auto it = insn->outputs_.begin(); it != insn->outputs_.end(); ++it) {
    IRegister *reg = *it;
    if (reg->IsConst() || reg->IsStateLocal()) {
      continue;
    }
    IRegister *wire_reg =
      new IRegister(table_, reg->GetName() + "_" + Util::Itoa(insn->GetId()));
    wire_reg->SetStateLocal(true);
    wire_reg->value_type_ = reg->value_type_;
    table_->registers_.push_back(wire_reg);
    AddWireToRegMapping(insn, wire_reg, reg);
    *it = wire_reg;
  }
}

void WireInsn::AddWireToRegMapping(IInsn *insn, IRegister *wire,
				   IRegister *reg) {
  PerInsn *pi = GetPerInsn(insn);
  pi->wire_to_register_[wire] = reg;
  pi->register_to_wire_[reg] = wire;
}

void WireInsn::ScanBBToMoveInsn(BB *bb) {
  for (int target = 0; target < bb->states_.size() - 1; ++target) {
    for (int pos = target + 1; pos < bb->states_.size(); ++pos) {
      vector<IInsn *> movable_insns;
      IState *src_st = bb->states_[pos];
      for (IInsn *insn : src_st->insns_) {
	if (CanMoveInsn(insn, bb, target)) {
	  movable_insns.push_back(insn);
	}
      }
      for (IInsn *insn : movable_insns) {
	MoveInsn(insn, bb, target);
      }
    }
  }
}

void WireInsn::MoveLastTransitionInsn(BB *bb) {
  IState *last_st = bb->states_[bb->states_.size() - 1];
  IInsn *last_tr_insn = DesignUtil::FindInsnByResource(last_st, transition_);
  if (last_tr_insn == nullptr ||
      last_tr_insn->target_states_.size() < 2) {
    // last transition is not a conditional.
    return;
  }
  int last_non_tr_insn_idx = 0;
  for (int i = 0; i < bb->states_.size() - 1; ++i) {
    IState *st = bb->states_[i];
    for (IInsn *insn : st->insns_) {
      if (insn->GetResource() != transition_) {
	last_non_tr_insn_idx = i;
	break;
      }
    }
  }
  if (last_non_tr_insn_idx == bb->states_.size() - 1) {
    // Last state has non transition insn, so nothing to do.
    return;
  }
  // Truncate unnecessry transitions after last state.
  for (int i = last_non_tr_insn_idx; i < bb->states_.size() - 1; ++i) {
    IState *st = bb->states_[i];
    vector<IInsn *> insns;
    for (IInsn *insn : st->insns_) {
      if (insn->GetResource() != transition_) {
	insns.push_back(insn);
      }
    }
    st->insns_ = insns;
  }
  // Move.
  IState *new_last_st = bb->states_[last_non_tr_insn_idx];
  new_last_st->insns_.push_back(last_tr_insn);
  bb->states_[bb->states_.size() - 1]->insns_.clear();
  // May rewrite the condition.
  CHECK(last_tr_insn->inputs_.size() == 1);
  IRegister *cond = last_tr_insn->inputs_[0];
  if (cond->IsConst()) {
    return;
  }
  CHECK(!cond->IsStateLocal());
  for (IInsn *insn : new_last_st->insns_) {
    if (insn->GetResource() != assign_) {
      continue;
    }
    if (insn->outputs_[0] == cond) {
      last_tr_insn->inputs_[0] = insn->inputs_[0];
    }
  }
}

bool WireInsn::CanMoveInsn(IInsn *insn, BB *bb, int target_pos) {
  if (insn->GetResource() == transition_) {
    return false;
  }
  IState *target_st = bb->states_[target_pos];
  if (!CanUseResourceInState(target_st, insn->GetResource())) {
    return false;
  }
  PerInsn *pi = GetPerInsn(insn);
  int max_pos = 0;
  // Check input dependency
  for (auto it : pi->depending_insn_) {
    PerInsn *src_pi = GetPerInsn(it.second);
    if (src_pi->nth_state > max_pos) {
      max_pos = src_pi->nth_state;
    }
  }
  if (max_pos <= target_pos) {
    return true;
  }
  return false;
}

void WireInsn::MoveInsn(IInsn *insn, BB *bb, int target_pos) {
  PerInsn *pi = GetPerInsn(insn);
  IState *src_st = bb->states_[pi->nth_state];
  IState *dst_st = bb->states_[target_pos];
  DesignTool::MoveInsn(insn, src_st, dst_st);
  pi->nth_state = target_pos;
  for (auto it = insn->inputs_.begin(); it != insn->inputs_.end(); ++it) {
    IRegister *ireg = *it;
    IInsn *src_insn = pi->depending_insn_[ireg];
    if (!src_insn) {
      continue;
    }
    PerInsn *src_pi = GetPerInsn(src_insn);
    if (src_pi->nth_state < target_pos) {
      // This register was an output in previous state,
      // so the wire is not avaialble.
      continue;
    }
    IRegister *alt_input = src_pi->register_to_wire_[ireg];
    if (alt_input == nullptr && src_pi->is_assign) {
      alt_input = *(src_insn->inputs_.begin());
    }
    if (alt_input != nullptr) {
      *it = alt_input;
    }
  }
}

bool WireInsn::CanUseResourceInState(IState *st, IResource *resource) {
  for (IInsn *target_insn : st->insns_) {
    if (resource->GetClass()->IsExclusive() &&
	resource == target_insn->GetResource()) {
      return false;
    }
  }
  return true;
}

void WireInsn::AddWireToRegisterAssignments() {
  for (IState *st : table_->states_) {
    vector<IInsn *> new_assign_insn;
    for (IInsn *insn : st->insns_) {
      PerInsn *pi = GetPerInsn(insn);
      for (IRegister *maybe_wire : insn->outputs_) {
	IRegister *orig_reg = pi->wire_to_register_[maybe_wire];
	if (orig_reg) {
	  const bool used_later = IsUsedLaterInThisBB(insn, orig_reg);
	  const bool reach_to_other_bb =
	    pi->output_reach_to_other_bb_.find(orig_reg) !=
	    pi->output_reach_to_other_bb_.end();
	  if (used_later || reach_to_other_bb) {
	    IInsn *assign_insn = new IInsn(assign_);
	    assign_insn->inputs_.push_back(maybe_wire);
	    assign_insn->outputs_.push_back(orig_reg);
	    new_assign_insn.push_back(assign_insn);
	  }
	}
      }
    }
    for (IInsn *insn : new_assign_insn) {
      st->insns_.push_back(insn);
    }
  }
}

bool WireInsn::IsUsedLaterInThisBB(IInsn *insn, IRegister *output) {
  PerInsn *pi = GetPerInsn(insn);
  map<IRegister *, set<IInsn *> >::iterator it = pi->using_insns_.find(output);
  if (it == pi->using_insns_.end()) {
    return false;
  }
  set<IInsn *> &users = it->second;
  for (IInsn *insn : users) {
    PerInsn *user_pi = GetPerInsn(insn);
    if (user_pi->nth_state > pi->nth_state) {
      return true;
    }
  }
  return false;
}

WireInsn::PerInsn *WireInsn::GetPerInsn(IInsn *insn) {
  PerInsn *pi = per_insn_map_[insn];
  if (pi == nullptr) {
    pi = new PerInsn;
    per_insn_map_[insn] = pi;
  }
  return pi;
}

}  // namespace wire
}  // namespace opt
}  // namespace iroha
