// -*- C++ -*-
#ifndef _opt_bb_collector_h_
#define _opt_bb_collector_h_

#include "iroha/common.h"
#include "opt/opt_util.h"

#include <set>

namespace iroha {
namespace opt {

class BB;
class BBSet;

class BBCollector {
public:
  BBCollector(ITable *table);
  BBSet *Create();

private:
  void CollectEntries();
  void CollectBBEntry(IState *es);
  void CollectBB(IState *es, IState *next_st);
  IState *GetOneNextState(IState *cur);

  ITable *table_;
  BBSet *bbs_;
  IResource *tr_;
  set<IState *> bb_entries_;
  map<IState *, TransitionInfo> transition_info_;
};

}  // namespace opt
}  // namespace iroha

#endif  // _opt_bb_collector_h_
