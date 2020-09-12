#include "opt/data_flow.h"

#include "iroha/stl_util.h"
#include "opt/data_flow_collector.h"

namespace iroha {
namespace opt {

DataFlow::~DataFlow() { STLDeleteValues(&all_defs_); }

DataFlow *DataFlow::Create(BBSet *bbs, OptimizerLog *opt_log) {
  DataFlowCollector collector(bbs, opt_log);
  return collector.Create();
}

void DataFlow::GetReachDefs(BB *bb, vector<RegDef *> *defs) {
  auto r = reaches_.equal_range(bb);
  for (auto it = r.first; it != r.second; ++it) {
    defs->push_back(it->second);
  }
}

}  // namespace opt
}  // namespace iroha
