// -*- C++ -*-
#ifndef _platform_platform_h_
#define _platform_platform_h_

#include "iroha/common.h"

namespace iroha {
namespace platform {

class Platform {
public:
  static void ResolvePlatform(IDesign *design);
};

}  // namespace platform
}  // namespace iroha

#endif  // _platform_platform_h_
