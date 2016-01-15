#include "opt/data_flow.h"

#include "opt/data_flow_collector.h"

namespace iroha {
namespace opt {

DataFlow *DataFlow::Create(BBSet *bbs, DebugAnnotation *annotation) {
  DataFlowCollector collector(bbs, annotation);
  return collector.Create();
}

void DataFlow::GetReachDefs(BB *bb, vector<RegDef *> *defs) {
}

}  // namespace opt
}  // namespace iroha
