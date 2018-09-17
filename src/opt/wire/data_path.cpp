#include "opt/wire/data_path.h"

#include "iroha/stl_util.h"
#include "opt/bb_set.h"

namespace iroha {
namespace opt {
namespace wire {

DataPath::DataPath(BB *bb) : bb_(bb) {
}

void DataPath::Build() {
}

DataPathSet::DataPathSet() {
}

DataPathSet::~DataPathSet() {
  STLDeleteSecondElements(&data_pathes_);
}

void DataPathSet::Build(BBSet *bset) {
  bbs_ = bset;
  for (BB *bb : bbs_->bbs_) {
    DataPath *dp = new DataPath(bb);
    data_pathes_[bb] = dp;
    dp->Build();
  }
}

}  // namespace wire
}  // namespace opt
}  // namespace iroha
