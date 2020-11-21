// -*- C++ -*-
#ifndef _opt_ssa_phi_cleaner_h_
#define _opt_ssa_phi_cleaner_h_

#include "opt/pass.h"

namespace iroha {
namespace opt {
namespace ssa {

class PhiCleaner {
 public:
  PhiCleaner(ITable *table, OptimizerLog *opt_log);
  ~PhiCleaner();

  void Perform();

 private:
  void ProcessBB(BB *bb);
  void ProcessInsn(map<IRegister *, RegDef *> *last_defs, IState *st,
                   IInsn *insn);
  void ProcessPhiInsn(map<IRegister *, RegDef *> *last_defs, IState *st,
                      IInsn *insn);
  void EmitSelector(map<IRegister *, RegDef *> *last_defs, IState *st,
                    IInsn *phi_insn);

  ITable *table_;
  OptimizerLog *opt_log_;
  unique_ptr<BBSet> bset_;
  unique_ptr<DataFlow> data_flow_;
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
