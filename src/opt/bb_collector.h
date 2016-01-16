// -*- C++ -*-
#ifndef _opt_bb_collector_h_
#define _opt_bb_collector_h_

#include "opt/common.h"
#include "opt/opt_util.h"

namespace iroha {
namespace opt {

class BB;
class BBSet;
class DebugAnnotation;

class BBCollector {
public:
  BBCollector(ITable *table, DebugAnnotation *annotation);
  BBSet *Create();

private:
  // Collect BBs.
  void CollectEntries();
  void CollectBBsFromEntry(IState *entry_st);
  void CollectBB(IState *entry_st, IState *next_st);
  IState *GetOneNextState(IState *cur);
  // Additional info.
  void SetStateBBMap();
  void SetBBTransition();
  void Annotate();

  ITable *table_;
  DebugAnnotation *annotation_;
  BBSet *bset_;
  IResource *tr_;
  set<IState *> bb_entries_;
  map<IState *, TransitionInfo> transition_info_;
};

}  // namespace opt
}  // namespace iroha

#endif  // _opt_bb_collector_h_
