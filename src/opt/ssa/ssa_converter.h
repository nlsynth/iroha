// -*- C++ -*-
#ifndef _opt_ssa_ssa_converter_h_
#define _opt_ssa_ssa_converter_h_

#include "opt/phase.h"

namespace iroha {
namespace opt {
namespace ssa {

class SSAConverter {
public:
  SSAConverter(ITable *table, DebugAnnotation *annotation);
  ~SSAConverter();
  void Perform();

private:
  class PerRegister {
  public:
    set<RegDef *> original_defs_;
    set<BB *> phi_bbs_;
  };

  void CollectSingularRegister();
  void CollectOriginalDefs();
  PerRegister *GetPerRegister(IRegister *reg);
  void PropagatePHIs();
  void PropagatePHIforBB(PerRegister *pr, BB *bb);
  void CommitPHIInsn();
  void PrependState(BB *bb);

  ITable *table_;
  DebugAnnotation *annotation_;
  IResource *phi_;
  IResource *tr_;
  BBSet *bset_;
  DataFlow *data_flow_;
  DominatorTree *dom_tree_;
  set<IRegister *> singular_regs_;
  map<IRegister *, PerRegister *> reg_phis_map_;
};

}  // namespace ssa
}  // namespace opt
}  // namespace iroha

#endif  // _opt_ssa_ssa_converter_h_
