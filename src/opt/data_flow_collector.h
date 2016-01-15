// -*- C++ -*-
#ifndef _opt_data_flow_collector_h_
#define _opt_data_flow_collector_h_

#include "opt/common.h"

#include <map>

namespace iroha {
namespace opt {

class DataFlowCollector {
public:
  DataFlowCollector(BBSet *bbs, DebugAnnotation *annotation);
  DataFlow *Create();

private:
  class BBInfo {
  public:
    map<IRegister *, RegDef *> last_defs_;
  };
  map<BB *, BBInfo *> bb_info_;
  BBSet *bbs_;
  DebugAnnotation *annotation_;
};

}  // namespace opt
}  // namespace iroha

#endif  // _opt_data_flow_collector_h_
