// -*- C++ -*-
#ifndef _opt_constant_constant_propagation_h_
#define _opt_constant_constant_propagation_h_

#include "opt/phase.h"

namespace iroha {
namespace opt {
namespace constant {

// This assumes SSA input and actually propagates non constant regs too.
class ConstantPropagation : public Phase {
public:
  virtual ~ConstantPropagation();

  static Phase *Create();

private:
  virtual bool ApplyForTable(const string &key, ITable *table);
};

}  // namespace constant
}  // namespace opt
}  // namespace iroha

#endif // _opt_constant_constant_propagation_h_
