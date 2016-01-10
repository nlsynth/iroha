// -*- C++ -*-
#ifndef _iroha_opt_api_h_
#define _iroha_opt_api_h_

#include "iroha/common.h"

namespace iroha {

class OptAPI {
public:
  virtual ~OptAPI();
  virtual bool ApplyPhase(const string &name) = 0;
  virtual void EnableDebugAnnotation() = 0;
};

}  // namespace iroha

#endif  // _iroha_opt_api_h_
