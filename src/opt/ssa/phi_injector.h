// -*- C++ -*-
#ifndef _opt_ssa_phi_injector_h_
#define _opt_ssa_phi_injector_h_

#include "opt/common.h"

namespace iroha {
namespace opt {
namespace ssa {

class PhiInjector {
 public:
  PhiInjector(ITable *table, OptimizerLog *opt_log);
  ~PhiInjector();

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
  OptimizerLog *opt_log_;
  IResource *phi_;
  IResource *tr_;
  unique_ptr<BBSet> bset_;
  unique_ptr<DataFlow> data_flow_;
  unique_ptr<DominatorTree> dom_tree_;
  set<IRegister *> singular_regs_;
  map<IRegister *, PerRegister *> reg_phis_map_;
};

}  // namespace ssa
}  // namespace opt
}  // namespace iroha

#endif  // _opt_ssa_phi_injector_h_
