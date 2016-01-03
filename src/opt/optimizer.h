// -*- C++ -*-
#ifndef _iroha_opt_optimizer_h_
#define _iroha_opt_optimizer_h_

#include "iroha/opt_api.h"
#include "opt/phase.h"

namespace iroha {

class IDesign;

class Optimizer : public OptAPI {
public:
  Optimizer(IDesign *design);

  static void Init();
  static void RegisterPhase(Phase *phase);

  virtual bool ApplyPhase(const string &name) override;

protected:
  IDesign *design_;

  static vector<Phase *> phases_;
};

}  // namespace iroha

#endif  // _iroha_opt_optimizer_h_
