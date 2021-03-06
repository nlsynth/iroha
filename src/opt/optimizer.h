// -*- C++ -*-
#ifndef _opt_optimizer_h_
#define _opt_optimizer_h_

#include <functional>
#include <map>

#include "iroha/opt_api.h"
#include "opt/pass.h"

namespace iroha {
namespace opt {

class Optimizer : public OptAPI {
 public:
  Optimizer(IDesign *design);
  ~Optimizer();

  static void Init();
  static vector<string> GetPassNames();
  static void RegisterPass(const string &name, function<Pass *()> factory);

  virtual bool ApplyPass(const string &name) override;
  virtual void EnableOptimizerLog() override;
  virtual void DumpIntermediateToFiles(const string &fn) override;

  platform::PlatformDB *GetPlatformDB();

 protected:
  void DumpTables();

  IDesign *design_;
  std::unique_ptr<platform::PlatformDB> platform_db_;

  static map<string, function<Pass *()> > passes_;
};

}  // namespace opt
}  // namespace iroha

#endif  // _opt_optimizer_h_
