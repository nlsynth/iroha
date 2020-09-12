#include "opt/bb_set.h"

#include <algorithm>

#include "iroha/stl_util.h"
#include "opt/bb_collector.h"
#include "opt/optimizer_log.h"

namespace {
struct LessBBP {
  bool operator()(iroha::opt::BB *l, iroha::opt::BB *r) const {
    return l->bb_id_ < r->bb_id_;
  }
};
}  // namespace

namespace iroha {
namespace opt {

BB::BB() : bb_id_(-1) {}

BBSet::BBSet(ITable *table) : initial_bb_(nullptr), table_(table) {}

BBSet::~BBSet() { STLDeleteValues(&bbs_); }

BBSet *BBSet::Create(ITable *table, bool splitMultiCycle,
                     OptimizerLog *opt_log) {
  BBCollector collector(table, splitMultiCycle, opt_log);
  return collector.Create();
}

void BBSet::SortBBs(const set<BB *> &input, vector<BB *> *sorted) {
  sorted->clear();
  for (BB *bb : input) {
    sorted->push_back(bb);
  }
  sort(sorted->begin(), sorted->end(), LessBBP());
}

void BBSet::Log(OptimizerLog *opt_log) {
  for (int i = 0; i < bbs_.size(); ++i) {
    int id = bbs_[i]->bb_id_;
    int idx = 0;
    for (IState *st : bbs_[i]->states_) {
      opt_log->State(st) << "bb:" << id << ":" << idx;
      opt_log->SetStateColorIndex(st, i);
      ++idx;
    }
  }
}

ITable *BBSet::GetTable() const { return table_; }

}  // namespace opt
}  // namespace iroha
