#include "opt/wire/simple_shrink.h"

#include "design/design_util.h"
#include "design/design_tool.h"
#include "design/resource_attr.h"
#include "iroha/i_design.h"
#include "iroha/logging.h"
#include "iroha/resource_class.h"
#include "opt/bb_set.h"
#include "opt/data_flow.h"
#include "opt/debug_annotation.h"
#include "opt/delay_info.h"

namespace iroha {
namespace opt {
namespace wire {

SimpleShrink::SimpleShrink(ITable *table, DebugAnnotation *annotation)
  : Scaffold(table, annotation) {
}

SimpleShrink::~SimpleShrink() {
}

bool SimpleShrink::Perform() {
  SetUp();

  for (BB *bb : bset_->bbs_) {
    ReplaceInsnOutputWithWireBB(bb);
    ScanBBToMoveInsn(bb);
  }
  AddWireToRegisterAssignments();
  for (BB *bb : bset_->bbs_) {
    MoveLastTransitionInsn(bb);
  }
  if (annotation_->IsEnabled()) {
    annotation_->DumpIntermediateTable(table_);
  }
  return true;
}

void SimpleShrink::ReplaceInsnOutputWithWireBB(BB *bb) {
  for (IState *st : bb->states_) {
    for (IInsn *insn : st->insns_) {
      // TODO(yt76): Don't touch multi cycles insns like memory ops.
      if (IsSimpleAssign(insn)) {
        PerInsn *pi = GetPerInsn(insn);
        pi->is_simple_assign = true;
      } else {
        ReplaceInsnOutputWithWire(insn);
      }
    }
  }
}

bool SimpleShrink::IsSimpleAssign(IInsn *insn) {
  if (insn->GetResource() != assign_) {
    return false;
  }
  if (insn->inputs_[0]->value_type_.GetWidth() !=
      insn->outputs_[0]->value_type_.GetWidth()) {
    return false;
  }
  return true;
}

void SimpleShrink::ReplaceInsnOutputWithWire(IInsn *insn) {
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

void SimpleShrink::AddWireToRegMapping(IInsn *insn, IRegister *wire,
				   IRegister *reg) {
  PerInsn *pi = GetPerInsn(insn);
  pi->wire_to_register_[wire] = reg;
  pi->register_to_wire_[reg] = wire;
}

void SimpleShrink::ScanBBToMoveInsn(BB *bb) {
  int target_pos = 0;
  do {
    target_pos = TryToMoveInsnsToTarget(bb, target_pos);
  } while (target_pos < bb->states_.size() - 1);
}

int SimpleShrink::TryToMoveInsnsToTarget(BB *bb, int target_pos) {
  int next_target = target_pos + 1;
  IState *target_st = bb->states_[target_pos];
  if (!IsSimpleState(target_st)) {
    return next_target;
  }
  for (int src_pos = target_pos + 1;
       src_pos < bb->states_.size(); ++src_pos) {
    IState *src_st = bb->states_[src_pos];
    if (!IsSimpleState(src_st)) {
      return src_pos + 1;
    }
    vector<MoveStrategy> movable_insns;
    for (IInsn *insn : src_st->insns_) {
      MoveStrategy ms;
      if (CanMoveInsn(insn, bb, target_pos, &ms)) {
	movable_insns.push_back(ms);
      }
    }
    TryMoveInsns(movable_insns, bb, target_pos, true);
    TryMoveInsns(movable_insns, bb, target_pos, false);
  }
  return next_target;
}

void SimpleShrink::TryMoveInsns(vector<MoveStrategy> &movable_insns, BB *bb,
				int target_pos, bool use_same_resource) {
  for (MoveStrategy &ms : movable_insns) {
    if (ms.use_same_resource == use_same_resource) {
      MoveInsn(&ms, bb, target_pos);
    }
  }
}

bool SimpleShrink::IsSimpleState(IState *st) {
  for (IInsn *insn : st->insns_) {
    if (ResourceAttr::IsExtAccessInsn(insn)) {
      return false;
    }
    if (ResourceAttr::IsExtWaitInsn(insn)) {
      return false;
    }
  }
  return true;
}

void SimpleShrink::MoveLastTransitionInsn(BB *bb) {
  IState *last_st = bb->states_[bb->states_.size() - 1];
  IInsn *last_tr_insn = DesignUtil::FindInsnByResource(last_st, transition_);
  if (last_tr_insn == nullptr ||
      last_tr_insn->target_states_.size() < 2) {
    // last transition is not conditional.
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

bool SimpleShrink::CanMoveInsn(IInsn *insn, BB *bb, int target_pos,
			       MoveStrategy *ms) {
  ms->insn = insn;
  if (insn->GetResource() == transition_) {
    return false;
  }
  IState *target_st = bb->states_[target_pos];
  if (ResourceAttr::IsExtAccessInsn(insn) &&
      ResourceAttr::NumExtAccessInsn(target_st) > 0) {
    return false;
  }
  if (!CanUseResourceInState(target_st, insn->GetResource(), ms)) {
    return false;
  }
  PerInsn *pi = GetPerInsn(insn);
  int max_pos = 0;
  // Checks input dependency by the position of insns.
  for (auto it : pi->depending_insn_) {
    PerInsn *src_pi = GetPerInsn(it.second);
    if (src_pi->nth_state > max_pos) {
      max_pos = src_pi->nth_state;
    }
  }
  if (max_pos > target_pos) {
    return false;
  }
  // Checks specified insn
  for (IInsn *dep : insn->depending_insns_) {
    PerInsn *dep_pi = GetPerInsn(dep);
    if (dep_pi->nth_state >= target_pos) {
      return false;
    }
  }
  // Checks the delay.
  if (!CheckDelay(insn, target_st)) {
    return false;
  }
  return true;
}

bool SimpleShrink::CheckDelay(IInsn *insn, IState *target_st) {
  int min_slack = -1;
  DelayInfo lat_info(0);
  for (IRegister *ireg : insn->inputs_) {
    int s = lat_info.GetRegisterSlack(target_st, ireg);
    if (min_slack < 0 || s < min_slack) {
      min_slack = s;
    }
  }
  if (min_slack < 0) {
    return true;
  }
  int lat = lat_info.GetInsnLatency(insn);
  if (min_slack < lat) {
    return false;
  }
  return true;
}

void SimpleShrink::MoveInsn(MoveStrategy *ms, BB *bb, int target_pos) {
  IInsn *insn = ms->insn;
  PerInsn *pi = GetPerInsn(insn);
  IState *src_st = bb->states_[pi->nth_state];
  pi->nth_state = target_pos;
  IState *dst_st = bb->states_[target_pos];
  IInsn *tmp_insn = MayCopyInsnForState(dst_st, insn, ms);
  if (tmp_insn == nullptr) {
    // overbooked.
    return;
  }
  if (tmp_insn == insn) {
    DesignTool::MoveInsn(insn, src_st, dst_st);
  } else {
    // new insn was allocated to use a different resource.
    DesignTool::EraseInsn(src_st, insn);
    insn = tmp_insn;
  }
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
    CHECK(src_pi->nth_state == target_pos);
    // Tries 2 sources to to shortcut the input.
    // (1) Direct output wire from the source insn.
    // (2) RHS of the assign, if the source is a simple assign insn.
    IRegister *alt_input = src_pi->register_to_wire_[ireg];
    if (alt_input == nullptr && src_pi->is_simple_assign) {
      alt_input = *(src_insn->inputs_.begin());
    }
    if (alt_input != nullptr) {
      *it = alt_input;
    }
  }
}

bool SimpleShrink::CanUseResourceInState(IState *st, IResource *resource,
					 MoveStrategy *ms) {
  for (IInsn *target_insn : st->insns_) {
    IResource *insn_resource = target_insn->GetResource();
    if (resource->GetClass()->IsExclusive() &&
	resource == insn_resource) {
      return false;
    }
  }
  ms->use_same_resource = true;
  return true;
}

IInsn *SimpleShrink::MayCopyInsnForState(IState *st, IInsn *insn,
					 MoveStrategy *ms) {
  return insn;
}

void SimpleShrink::AddWireToRegisterAssignments() {
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

bool SimpleShrink::IsUsedLaterInThisBB(IInsn *insn, IRegister *output) {
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

}  // namespace wire
}  // namespace opt
}  // namespace iroha
