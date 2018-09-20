// -*- C++ -*-
#ifndef _opt_optimizer_h_
#define _opt_optimizer_h_

#include "iroha/opt_api.h"
#include "opt/phase.h"

#include <functional>
#include <map>

namespace iroha {
namespace opt {

class Optimizer : public OptAPI {
public:
  Optimizer(IDesign *design);

  static void Init();
  static void RegisterPhase(const string &name,
			    function<Phase *()> factory);

  virtual bool ApplyPhase(const string &name) override;
  virtual void EnableDebugAnnotation() override;
  virtual void DumpIntermediateToFiles(const string &fn) override;

protected:
  IDesign *design_;

  static map<string, function<Phase *()> > phases_;
};

}  // namespace opt
}  // namespace iroha

#endif  // _opt_optimizer_h_
