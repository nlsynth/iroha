// -*- C++ -*-
#ifndef _opt_ssa_phi_cleaner_h_
#define _opt_ssa_phi_cleaner_h_

#include "opt/pass.h"

namespace iroha {
namespace opt {
namespace ssa {

class ConditionValueRange;

class PhiCleaner {
 public:
  PhiCleaner(ITable *table, OptimizerLog *opt_log);
  ~PhiCleaner();

  void Perform();

 private:
  void ProcessBB(BB *bb);
  void ProcessInsn(map<IRegister *, RegDef *> *last_defs, IState *st,
                   IInsn *insn);
  void ProcessPhiInsn(map<IRegister *, RegDef *> *last_defs, IState *phi_st,
                      IInsn *phi_insn);
  void EmitSelector(map<IRegister *, RegDef *> *last_defs, IState *phi_st,
                    IInsn *phi_insn);
  IRegister *GetConditionReg(map<IRegister *, RegDef *> *last_defs,
                             IInsn *phi_insn, bool *order01);

  ITable *table_;
  OptimizerLog *opt_log_;
  unique_ptr<BBSet> bset_;
  unique_ptr<DataFlow> data_flow_;
  unique_ptr<ConditionValueRange> cv_range_;
  IResource *phi_;
  IResource *sel_;
  IResource *assign_;
  int nth_sel_;
  map<IInsn *, set<RegDef *> > reg_def_map_;
};

}  // namespace ssa
}  // namespace opt
}  // namespace iroha

#endif  // _opt_ssa_phi_cleaner_h_
