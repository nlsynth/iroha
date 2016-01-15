// -*- C++ -*-
#ifndef _opt_data_flow_h_
#define _opt_data_flow_h_

#include "opt/common.h"

namespace iroha {
namespace opt {

class RegDef {
public:
  IRegister *reg;
  IInsn *insn;
  int output_index;
  IState *st;
  BB *bb;
};

class DataFlow {
public:
  static DataFlow *Create(BBSet *bbs, DebugAnnotation *annotation);

  void GetReachDefs(BB *bb, vector<RegDef *> *defs);
};

}  // namespace opt
}  // namespace iroha

#endif  // _opt_data_flow_h_
