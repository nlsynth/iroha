// -*- C++ -*-
#ifndef _platform_platform_db_h_
#define _platform_platform_db_h_

#include "iroha/common.h"

namespace iroha {
namespace platform {

class PlatformDB {
public:
  PlatformDB(IPlatform *platform);

private:
  IPlatform *platform_;
};

}  // namespace platform
}  // namespace iroha

#endif  // _platform_platform_db_h_

