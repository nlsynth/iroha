// -*- C++ -*-
#ifndef _iroha_opt_optimizer_h_
#define _iroha_opt_optimizer_h_

#include "iroha/opt_api.h"

namespace iroha {

class IDesign;

class Optimizer : public OptAPI {
public:
  Optimizer(IDesign *design);
  virtual bool ApplyPhase(const string &name) override;

private:
  IDesign *design_;
};

}  // namespace iroha

#endif  // _iroha_opt_optimizer_h_
