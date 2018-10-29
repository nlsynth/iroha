// -*- C++ -*-
#ifndef _platform_platform_db_h_
#define _platform_platform_db_h_

#include "iroha/common.h"

namespace iroha {
namespace platform {

class LookupCondition;

// This holds a result value of node evaluation.
class NodeResult {
public:
  NodeResult(bool b);
  NodeResult(int v);

  bool IsBool() const;
  bool BoolVal() const;
  int IntVal() const;

private:
  bool is_bool_;
  bool bv_;
  bool iv_;
};

class PlatformDB {
public:
  PlatformDB(IPlatform *platform);

  int GetResourceDelay(IResource *res);

private:
  DefNode *FindValue(const LookupCondition &lookup_cond);
  NodeResult MatchCond(const LookupCondition &lookup_cond, DefNode *cond_node);
  int GetInt(DefNode *node, const string &key, int dflt);

  IPlatform *platform_;
};

}  // namespace platform
}  // namespace iroha

#endif  // _platform_platform_db_h_

