#include "opt/data_flow_collector.h"

#include "iroha/i_design.h"
#include "opt/bb_set.h"
#include "opt/data_flow.h"
#include "opt/debug_annotation.h"

namespace iroha {
namespace opt {

DataFlowCollector::DataFlowCollector(BBSet *bbs, DebugAnnotation *annotation)
  : bbs_(bbs), annotation_(annotation) {
}

DataFlow *DataFlowCollector::Create() {
  for (auto *bb : bbs_->bbs_) {
    BBInfo *info = new BBInfo;
    info->bb_ = bb;
    bb_info_[bb] = info;
  }
  df_ = new DataFlow;
  for (auto p : bb_info_) {
    CollectDefs(p.second);
  }
  for (auto p : bb_info_) {
    CollectKills(p.second);
  }
  CollectReaches();
  CopyReaches();
  if (annotation_->IsEnabled()) {
    Annotate(annotation_->Table(bbs_->GetTable()));
  }
  return df_;
}

void DataFlowCollector::Annotate(ostream &os) {
  for (auto p : bb_info_) {
    os << "bb:" << p.first->bb_id_ << "<br>\n";
    for (auto *reg_def : p.second->reaches_) {
      os << " def: insn:" << reg_def->insn->GetId()
	 << " reg: " << reg_def->reg->GetName() << "<br>\n";
    }
  }
}

void DataFlowCollector::CollectDefs(BBInfo *info) {
  for (IState *st : info->bb_->states_) {
    for (IInsn *insn : st->insns_) {
      int nth_output = 0;
      for (IRegister *oreg : insn->outputs_) {
	if (!(oreg->IsConst() || oreg->IsStateLocal())) {
	  // Normal register.
	  RegDef *reg_def = new RegDef;
	  reg_def->reg = oreg;
	  reg_def->insn = insn;
	  reg_def->output_index = nth_output;
	  reg_def->st = st;
	  reg_def->bb = info->bb_;
	  df_->all_defs_.push_back(reg_def);
	  info->last_defs_[oreg] = reg_def;
	}
	++nth_output;
      }
    }
  }
}

void DataFlowCollector::CollectKills(BBInfo *info) {
  for (RegDef *reg_def : df_->all_defs_) {
    if (info->bb_ == reg_def->bb) {
      continue;
    }
    if (info->last_defs_.find(reg_def->reg) == info->last_defs_.end()) {
      continue;
    }
    info->kills_.push_back(reg_def);
  }
}

void DataFlowCollector::CollectReaches() {
  bool changed;
  do {
    changed = false;
    for (auto p : bb_info_) {
      BBInfo *info = p.second;
      set<RegDef *> temp;
      for (BB *prev_bb : info->bb_->prev_bbs_) {
	BBInfo *prev_bb_info = bb_info_[prev_bb];
	CollectPropagates(prev_bb_info, &temp);
      }
      if (temp.size() > info->reaches_.size()) {
	changed = true;
	info->reaches_ = temp;
      }
    }
  } while (changed);
}

void DataFlowCollector::CopyReaches() {
  for (auto p : bb_info_) {
    BBInfo *info = p.second;
    for (RegDef *reg_def : info->reaches_) {
      df_->reaches_.insert(make_pair(info->bb_, reg_def));
    }
  }
}

void DataFlowCollector::CollectPropagates(BBInfo *prev_bb_info,
					  set<RegDef *> *prop) {
  // DEF(P)
  for (auto e : prev_bb_info->last_defs_) {
    prop->insert(e.second);
  }
  // REACH(P) - KILL(P)
  set<RegDef *> defs;
  for (RegDef *r : prev_bb_info->reaches_) {
    defs.insert(r);
  }
  for (RegDef *k : prev_bb_info->kills_) {
    defs.erase(k);
  }
  for (RegDef *r : defs) {
    prop->insert(r);
  }
}

}  // namespace opt
}  // namespace iroha
