// -*- C++ -*-
#ifndef _opt_study_h_
#define _opt_study_h_

#include "opt/pass.h"

namespace iroha {
namespace opt {

class Study : public Pass {
 public:
  virtual ~Study();

  static Pass *Create();
};

}  // namespace opt
}  // namespace iroha

#endif  // _opt_study_h_
