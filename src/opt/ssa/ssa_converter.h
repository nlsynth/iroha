// -*- C++ -*-
#ifndef _opt_ssa_ssa_converter_h_
#define _opt_ssa_ssa_converter_h_

#include "opt/common.h"

namespace iroha {
namespace opt {
namespace ssa {

class SSAConverter {
 public:
  SSAConverter(ITable *table, OptimizerLog *opt_log);
  ~SSAConverter();

  void Perform();

 private:
  void InjectInitialValueAssigns();

  ITable *table_;
  OptimizerLog *opt_log_;
};

}  // namespace ssa
}  // namespace opt
}  // namespace iroha

#endif  // _opt_ssa_ssa_converter_h_
