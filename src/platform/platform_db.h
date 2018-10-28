// -*- C++ -*-
#ifndef _platform_platform_db_h_
#define _platform_platform_db_h_

#include "iroha/common.h"

namespace iroha {
namespace platform {

class LookupCondition;

class PlatformDB {
public:
  PlatformDB(IPlatform *platform);

  int GetResourceDelay(IResource *res);

private:
  DefNode *FindValue(const LookupCondition &lookup_cond);
  bool MatchCond(const LookupCondition &lookup_cond, DefNode *cond_node);
  int GetInt(DefNode *node, const string &key, int dflt);

  IPlatform *platform_;
};

}  // namespace platform
}  // namespace iroha

#endif  // _platform_platform_db_h_

