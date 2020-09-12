// -*- C++ -*-
#ifndef _opt_ssa_ssa_h_
#define _opt_ssa_ssa_h_

#include "opt/pass.h"

namespace iroha {
namespace opt {
namespace ssa {

class SSAConverterPass : public Pass {
 public:
  virtual ~SSAConverterPass();

  static Pass *Create();

 private:
  virtual bool ApplyForTable(const string &key, ITable *table);
};

class PhiCleanerPass : public Pass {
 public:
  virtual ~PhiCleanerPass();

  static Pass *Create();

 private:
  virtual bool ApplyForTable(const string &key, ITable *table);
};

}  // namespace ssa
}  // namespace opt
}  // namespace iroha

#endif  // _opt_ssa_ssa_h_
