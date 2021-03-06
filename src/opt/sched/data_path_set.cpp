#include "opt/sched/data_path_set.h"

#include "iroha/i_design.h"
#include "iroha/stl_util.h"
#include "opt/bb_set.h"
#include "opt/optimizer_log.h"
#include "opt/sched/bb_data_path.h"
#include "opt/sched/sched_block.h"
#include "opt/sched/virtual_resource_set.h"

namespace iroha {
namespace opt {
namespace sched {

DataPathSet::DataPathSet() {}

DataPathSet::~DataPathSet() { STLDeleteSecondElements(&data_paths_); }

void DataPathSet::Build(BBSet *bset) {
  vres_set_.reset(new VirtualResourceSet(bset->GetTable()));
  bbs_ = bset;
  sbs_.reset(new SchedBlockSet(bset));
  for (SchedBlock *sb : sbs_->fbs_) {
    BB *bb = sb->bb_;
    BBDataPath *dp = new BBDataPath(bb, vres_set_.get());
    data_paths_[bb->bb_id_] = dp;
    dp->Build();
  }
  vres_set_->BuildDefaultBinding();
}

void DataPathSet::SetDelay(DelayInfo *dinfo) {
  for (auto &p : data_paths_) {
    BBDataPath *dp = p.second;
    dp->SetDelay(dinfo);
  }
}

void DataPathSet::Dump(OptimizerLog *an) {
  ostream &os = an->GetDumpStream();
  os << "DataPathSet table: " << bbs_->GetTable()->GetId() << "\n";
  for (auto &p : data_paths_) {
    p.second->Dump(os);
  }
}

map<int, BBDataPath *> &DataPathSet::GetBBPaths() { return data_paths_; }

BBSet *DataPathSet::GetBBSet() { return bbs_; }

VirtualResourceSet *DataPathSet::GetVirtualResourceSet() {
  return vres_set_.get();
}

}  // namespace sched
}  // namespace opt
}  // namespace iroha
