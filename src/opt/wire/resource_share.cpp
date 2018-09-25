#include "opt/wire/resource_share.h"

#include "design/design_tool.h"
#include "design/resource_attr.h"
#include "iroha/i_design.h"
#include "iroha/stl_util.h"
#include "opt/bb_set.h"

namespace iroha {
namespace opt {
namespace wire {

class ResourceEntry {
public:
  ResourceEntry() : ires_(nullptr), duplicatable_(false), is_congested_(false) {
  }
  IResource *ires_;
  set<IInsn *> using_insns_;
  bool duplicatable_;
  bool is_congested_;
  // [0] is the original. Others are copies of it.
  vector<IResource *> all_res_;
  vector<int> usage_count_;
  int allocation_index_;
};

class BBEntry {
public:
  map<IResource *, int> usage_count_;
};

ResourceShare::ResourceShare(ITable *tab) : tab_(tab) {
}

ResourceShare::~ResourceShare() {
  STLDeleteSecondElements(&entries_);
  STLDeleteSecondElements(&bb_entries_);
}

void ResourceShare::Scan(BBSet *bbs) {
  // Populates resource entries_.
  for (IResource *res : tab_->resources_) {
    ResourceEntry *re = new ResourceEntry();
    re->ires_ = res;
    re->all_res_.push_back(res);
    re->duplicatable_ = ResourceAttr::IsDuplicatableResource(res);
    entries_[res] = re;
  }
  // Count the number of insns for each resource.
  for (IState *st : tab_->states_) {
    for (IInsn *insn : st->insns_) {
      ResourceEntry *re = entries_[insn->GetResource()];
      re->using_insns_.insert(insn);
    }
  }
  // Basic block.
  for (BB *bb : bbs->bbs_) {
    BBEntry *bbe = new BBEntry();
    bb_entries_[bb] = bbe;
    for (IState *st : bb->states_) {
      for (IInsn *insn : st->insns_) {
	IResource *res = insn->GetResource();
	bbe->usage_count_[res] += 1;
      }
    }
  }
  // Aggregate.
  CollectCongestedResource();
}

void ResourceShare::CollectCongestedResource() {
  // Congested in a BB.
  set<IResource *> congested;
  for (auto &p : bb_entries_) {
    BB *bb = p.first;
    BBEntry *bbe = p.second;
    for (auto &q : bbe->usage_count_) {
      if (q.second >= 2) {
	congested.insert(q.first);
      }
    }
  }
  // Globally congested.
  for (auto &p : entries_) {
    IResource *res = p.first;
    ResourceEntry *re = p.second;
    if (re->using_insns_.size() >= 3) {
      congested.insert(res);
    }
  }
  for (auto *ires : congested) {
    ResourceEntry *re = entries_[ires];
    if (!re->duplicatable_) {
      continue;
    }
    re->is_congested_ = true;
  }
}

void ResourceShare::Allocate() {
  for (auto &p : entries_) {
    ResourceEntry *re = p.second;
    if (!re->is_congested_) {
      continue;
    }
    // Allocates only 1 more resource for now.
    IResource *new_res = DesignTool::CopySimpleResource(re->ires_);
    re->all_res_.push_back(new_res);
  }
  for (auto &p : entries_) {
    ResourceEntry *re = p.second;
    if (re->all_res_.size() > 1) {
      int s = re->all_res_.size();
      re->usage_count_.resize(s);
      re->allocation_index_ = 0;
    }
  }
}

void ResourceShare::ReBind() {
  // Assign ids.
  for (auto &p : bb_entries_) {
    BB *bb = p.first;
    for (IState *st : bb->states_) {
      for (IInsn *insn : st->insns_) {
	IResource *res = insn->GetResource();
	ResourceEntry *re = entries_[res];
	if (re->usage_count_.size() > 0) {
	  AssignResourceForOneInsn(insn, re);
	}
      }
    }
  }
  for (auto &p : bb_entries_) {
    BB *bb = p.first;
    for (IState *st : bb->states_) {
      for (IInsn *insn : st->insns_) {
	auto q = rebind_index_.find(insn);
	if (q != rebind_index_.end() &&
	    q->second > 0) {
	  IResource *res = insn->GetResource();
	  ResourceEntry *re = entries_[res];
	  int idx = rebind_index_[insn];
	  IResource *new_res = re->all_res_[idx];
	  insn->SetResource(new_res);
	}
      }
    }
  }
}

void ResourceShare::AssignResourceForOneInsn(IInsn *insn, ResourceEntry *re) {
  int least_used_count = 10000;
  int least_used_index = -1;
  for (int i = 0; i < re->usage_count_.size(); ++i) {
    if (re->usage_count_[i] < least_used_count) {
      least_used_count = re->usage_count_[i];
      least_used_index = i;
    }
  }
  rebind_index_[insn] = least_used_index;
  re->usage_count_[least_used_index]++;
}

}  // namespace wire
}  // namespace opt
}  // namespace iroha
