// -*- C++ -*-
#ifndef _opt_ssa_ssa_h_
#define _opt_ssa_ssa_h_

#include "opt/phase.h"

namespace iroha {
namespace opt {
namespace ssa {

class SSAConverterPhase : public Phase {
public:
  virtual ~SSAConverterPhase();

  static Phase *Create();
private:
  virtual bool ApplyForTable(const string &key, ITable *table);
};

class PhiCleanerPhase : public Phase {
public:
  virtual ~PhiCleanerPhase();

  static Phase *Create();
private:
  virtual bool ApplyForTable(const string &key, ITable *table);
};

}  // namespace ssa
}  // namespace opt
}  // namespace iroha

#endif  // _opt_ssa_ssa_h_
