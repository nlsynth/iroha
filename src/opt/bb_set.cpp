#include "opt/bb_set.h"

#include "opt/bb_collector.h"
#include "opt/debug_annotation.h"
#include "iroha/stl_util.h"

#include <algorithm>

namespace {
struct LessBBP {
  bool operator()(iroha::opt::BB *l, iroha::opt::BB *r) const {
    return l->bb_id_ < r->bb_id_;
  }
};
}  // namespace

namespace iroha {
namespace opt {

BB::BB() : bb_id_(-1) {
}

BBSet::BBSet(ITable *table) : initial_bb_(nullptr), table_(table) {
}

BBSet::~BBSet() {
  STLDeleteValues(&bbs_);
}

BBSet *BBSet::Create(ITable *table, DebugAnnotation *annotation) {
  BBCollector collector(table, annotation);
  return collector.Create();
}

void BBSet::SortBBs(const set<BB *> &input, vector<BB *> *sorted) {
  sorted->clear();
  for (BB *bb : input) {
    sorted->push_back(bb);
  }
  sort(sorted->begin(), sorted->end(), LessBBP());
}

void BBSet::Annotate(DebugAnnotation *annotation) {
  for (int i = 0; i < bbs_.size(); ++i) {
    for (IState *st : bbs_[i]->states_) {
      annotation->State(st) << "bb:" << i;
    }
  }
}

ITable *BBSet::GetTable() const {
  return table_;
}

}  // namespace opt
}  // namespace iroha
