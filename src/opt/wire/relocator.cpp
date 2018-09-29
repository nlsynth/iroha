#include "opt/wire/relocator.h"

#include "iroha/i_design.h"
#include "opt/bb_set.h"
#include "opt/wire/data_path.h"

namespace iroha {
namespace opt {
namespace wire {

Relocator::Relocator(DataPathSet *data_path_set)
  : data_path_set_(data_path_set) {
}

void Relocator::Relocate() {
  auto &paths = data_path_set_->GetPaths();
  for (auto &p : paths) {
    RelocateInsnsForDataPath(p.second);
  }
}

void Relocator::RelocateInsnsForDataPath(DataPath *dp) {
  BB *bb = dp->GetBB();
  auto &edges = dp->GetEdges();
  // TODO: Handle transition insn.
  for (IState *st : bb->states_) {
    st->insns_.clear();
  }
  for (auto &p : edges) {
    PathEdge *e = p.second;
    bb->states_[e->final_st_index_]->insns_.push_back(e->insn_);
  }
}

}  // namespace wire
}  // namespace opt
}  // namespace iroha
