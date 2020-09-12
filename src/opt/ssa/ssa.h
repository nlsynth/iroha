// -*- C++ -*-
#ifndef _opt_ssa_ssa_h_
#define _opt_ssa_ssa_h_

#include "opt/pass.h"

namespace iroha {
namespace opt {
namespace ssa {

class SSAConverterPhase : public Pass {
 public:
  virtual ~SSAConverterPhase();

  static Pass *Create();

 private:
  virtual bool ApplyForTable(const string &key, ITable *table);
};

class PhiCleanerPhase : public Pass {
 public:
  virtual ~PhiCleanerPhase();

  static Pass *Create();

 private:
  virtual bool ApplyForTable(const string &key, ITable *table);
};

}  // namespace ssa
}  // namespace opt
}  // namespace iroha

#endif  // _opt_ssa_ssa_h_
