// -*- C++ -*-
#ifndef _opt_study_h_
#define _opt_study_h_

#include "opt/phase.h"

namespace iroha {
namespace opt {

class Study : public Phase {
public:
  virtual ~Study();

  static Phase *Create();
};

}  // namespace opt
}  // namespace iroha

#endif  // _opt_study_h_
