// -*- C++ -*-
#ifndef _opt_opt_util_h_
#define _opt_opt_util_h_

#include "iroha/common.h"

#include <map>
#include <set>

namespace iroha {
namespace opt {

class TransitionInfo {
public:
  TransitionInfo();

  int nr_branch_;
  int nr_join_;
};

class OptUtil {
public:
  static void CollectReachableStates(ITable *tab, set<IState *> *reachable);
  static void CollectReachableStatesWithExclude(ITable *tab,
						IState *initial,
						IState *exclude,
						set<IState *> *reachable);
  static void CollectTransitionInfo(ITable *tab,
				    map<IState *, TransitionInfo> *transition_info);
  static IState *GetOneNextState(IState *cur);

private:
  static void CollectTransitionTargets(ITable *tab,
				       map<IState *, set<IState *> > *targets);
};

}  // namespace opt
}  // namespace iroha

#endif  // _opt_opt_util_h_
