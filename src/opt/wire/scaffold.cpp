#include "opt/wire/scaffold.h"

#include "design/design_tool.h"
#include "design/design_util.h"
#include "iroha/stl_util.h"
#include "iroha/resource_class.h"
#include "opt/bb_set.h"
#include "opt/data_flow.h"
#include "opt/debug_annotation.h"

namespace iroha {
namespace opt {
namespace wire {

Scaffold::Scaffold(ITable *table, DebugAnnotation *annotation)
  : table_(table), annotation_(annotation) {
}

Scaffold::~Scaffold() {
  STLDeleteSecondElements(&per_insn_map_);
}

void Scaffold::SetUp() {
  assign_ = DesignTool::GetOneResource(table_, resource::kSet);
  transition_ = DesignUtil::FindTransitionResource(table_);
  bset_.reset(BBSet::Create(table_, annotation_));
  data_flow_.reset(DataFlow::Create(bset_.get(), annotation_));
  if (annotation_ != nullptr) {
    annotation_->DumpIntermediateTable(table_);
  }
  CollectReachingRegisters();
}

void Scaffold::CollectReachingRegisters() {
  CollectUsedRegsPerBB();
  // Collect defs used somewhere in this table.
  set<RegDef *> active_defs;
  for (BB *bb : bset_->bbs_) {
    vector<RegDef *> reach_defs;
    data_flow_->GetReachDefs(bb, &reach_defs);
    auto &bb_regs = used_regs_[bb];
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

void Scaffold::CollectUsedRegsPerBB() {
  for (IState *st : table_->states_) {
    BB *bb = bset_->state_to_bb_[st];
    for (IInsn *insn : st->insns_) {
      for (IRegister *ireg : insn->inputs_) {
	used_regs_[bb].insert(ireg);
      }
    }
  }
}

Scaffold::PerInsn *Scaffold::GetPerInsn(IInsn *insn) {
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
