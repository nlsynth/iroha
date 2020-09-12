// -*- C++ -*-
#ifndef _opt_data_flow_collector_h_
#define _opt_data_flow_collector_h_

#include "opt/common.h"

namespace iroha {
namespace opt {

class DataFlowCollector {
 public:
  DataFlowCollector(BBSet *bbs, OptimizerLog *opt_log);
  DataFlow *Create();

 private:
  class BBInfo {
   public:
    BB *bb_;
    map<IRegister *, RegDef *> last_defs_;
    vector<RegDef *> kills_;
    set<RegDef *> reaches_;
  };
  void CollectDefs(BBInfo *info);
  void CollectKills(BBInfo *info);
  void CollectReaches();
  void CollectPropagates(BBInfo *prev_bb_info, set<RegDef *> *prop);
  void CopyReaches();
  void Log(ostream &os);
  map<BB *, BBInfo *> bb_info_;
  BBSet *bbs_;
  DataFlow *df_;
  OptimizerLog *opt_log_;
};

}  // namespace opt
}  // namespace iroha

#endif  // _opt_data_flow_collector_h_
