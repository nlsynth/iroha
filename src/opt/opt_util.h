// -*- C++ -*-
#ifndef _opt_opt_util_h_
#define _opt_opt_util_h_

#include "iroha/common.h"

#include <map>
#include <set>

namespace iroha {
namespace opt {

class OptUtil {
public:
  static void CollectReachableStates(ITable *tab, set<IState *> *reachable);

private:
  static void CollectTransitionTargets(ITable *tab,
				       map<IState *, set<IState *>> *targets);
};

}  // namespace opt
}  // namespace iroha

#endif  // _opt_opt_util_h_
