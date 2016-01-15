#include "opt/data_flow_collector.h"

#include "opt/bb_set.h"
#include "opt/data_flow.h"

namespace iroha {
namespace opt {

DataFlowCollector::DataFlowCollector(BBSet *bbs, DebugAnnotation *annotation)
  : bbs_(bbs), annotation_(annotation) {
}

DataFlow *DataFlowCollector::Create() {
  for (auto *bb : bbs_->bbs_) {
    bb_info_[bb] = new BBInfo;
  }
  DataFlow *df = new DataFlow;
  return df;
}

}  // namespace opt
}  // namespace iroha
