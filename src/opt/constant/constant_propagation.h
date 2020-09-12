// -*- C++ -*-
#ifndef _opt_constant_constant_propagation_h_
#define _opt_constant_constant_propagation_h_

#include "opt/pass.h"

namespace iroha {
namespace opt {
namespace constant {

// This assumes SSA input and actually propagates non constant regs too.
class ConstantPropagation : public Pass {
 public:
  virtual ~ConstantPropagation();

  static Pass *Create();

 private:
  virtual bool ApplyForTable(const string &key, ITable *table);
};

}  // namespace constant
}  // namespace opt
}  // namespace iroha

#endif  // _opt_constant_constant_propagation_h_
