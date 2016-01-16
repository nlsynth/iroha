#include "opt/wire_insn.h"

#include "iroha/i_design.h"
#include "opt/bb_set.h"
#include "opt/data_flow.h"

namespace iroha {
namespace opt {

WireInsnPhase::~WireInsnPhase() {
}

Phase *WireInsnPhase::Create() {
  return new WireInsnPhase();
}

bool WireInsnPhase::ApplyForTable(ITable *table) {
  WireInsn wire_insn(table, annotation_);
  return wire_insn.Perform();
}

WireInsn::WireInsn(ITable *table, DebugAnnotation *annotation)
  : table_(table), annotation_(annotation), bset_(nullptr), data_flow_(nullptr) {
}

WireInsn::~WireInsn() {
  delete bset_;
  delete data_flow_;
}

bool WireInsn::Perform() {
  bset_ = BBSet::Create(table_, annotation_);
  data_flow_ = DataFlow::Create(bset_, annotation_);
  CollectReachingRegisters();
  for (BB *bb : bset_->bbs_) {
    BuildDependency(bb);
  }
  return true;
}

void WireInsn::CollectReachingRegisters() {
  CollectUsedRegs();
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

void WireInsn::CollectUsedRegs() {
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

void WireInsn::BuildRWDependencyPair(IInsn *insn, IRegister *reg,
				     map<IRegister *, IInsn *> &dep_map) {
  IRegister *input = reg;
  IInsn *def_insn = dep_map[input];
  if (!def_insn) {
    // not written/read in this block.
    return;
  }
  PerInsn *pi = GetPerInsn(insn);
  pi->depending_insn_[input] = def_insn;
  // adds reverse mapping too.
  PerInsn *def_insn_pi = GetPerInsn(def_insn);
  def_insn_pi->using_insns_[input].insert(insn);
}

WireInsn::PerInsn *WireInsn::GetPerInsn(IInsn *insn) {
  PerInsn *pi = per_insn_map_[insn];
  if (pi == nullptr) {
    pi = new PerInsn;
    per_insn_map_[insn] = pi;
  }
  return pi;
}

}  // namespace opt
}  // namespace iroha
