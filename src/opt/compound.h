// -*- C++ -*-
//
// Pre-defined pass sets.
//
#ifndef _opt_compound_h_
#define _opt_compound_h_

#include "opt/pass.h"

namespace iroha {
namespace opt {

class CompoundPass : public Pass {
 public:
  virtual ~CompoundPass();

  static void Init();
  static Pass *Create();

 private:
  virtual bool ApplyForDesign(IDesign *design);
};

}  // namespace opt
}  // namespace iroha

#endif  // _opt_compound_h_
