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
  NodeResult();
  NodeResult(bool b);
  NodeResult(int v);

  static NodeResult ErrorNode();

  bool IsBool() const;
  bool BoolVal() const;
  int IntVal() const;
  bool HasError() const;
  NodeResult SetError();

private:
  bool has_error_;
  bool is_bool_;
  bool bv_;
  int iv_;
};

class PlatformDB {
public:
  PlatformDB(IPlatform *platform);

  int GetResourceDelay(IResource *res);

private:
  DefNode *FindValue(const LookupCondition &lookup_cond);
  NodeResult EvalNode(const LookupCondition &lookup_cond, DefNode *cond_node);
  NodeResult EvalCompare(const LookupCondition &lookup_cond, DefNode *cond_node, bool isGt);
  int GetInt(DefNode *node, const string &key, int dflt);

  IPlatform *platform_;
};

}  // namespace platform
}  // namespace iroha

#endif  // _platform_platform_db_h_

