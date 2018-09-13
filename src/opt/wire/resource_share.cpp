#include "opt/wire/resource_share.h"

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
};

class BBEntry {
public:
  map<IResource *, int> use_count_;
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
	bbe->use_count_[res] += 1;
      }
    }
  }
  // Aggregate.
  CollectCongestedResource();
}

void ResourceShare::Allocate() {
}

void ResourceShare::CollectCongestedResource() {
  // Congested in a BB.
  set<IResource *> congested;
  for (auto &p : bb_entries_) {
    BB *bb = p.first;
    BBEntry *bbe = p.second;
    for (auto &q : bbe->use_count_) {
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

}  // namespace wire
}  // namespace opt
}  // namespace iroha
