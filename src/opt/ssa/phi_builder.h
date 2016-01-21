// -*- C++ -*-
#ifndef _opt_ssa_phi_builder_h_
#define _opt_ssa_phi_builder_h_

#include "opt/common.h"

namespace iroha {
namespace opt {
namespace ssa {

class PhiBuilder {
public:
  PhiBuilder(ITable *tab, DebugAnnotation *annotation);
  ~PhiBuilder();
  void Perform();

private:
  class PHI {
  public:
    IInsn *insn_;
    set<RegDef *> defs_;
  };
  class PerRegister {
  public:
    map<int,IRegister *> versions_;
  };

  void CalculatePHIInputsForBB(BB *bb);
  void UpdatePHIInputs(PHI *phi);
  void UpdateVersionsForBB(BB *bb);
  void UpdateVersionsForInsn(map<IRegister *, RegDef *> *last_defs,
			     IInsn *insn);
  // For versioning.
  int GetVersionFromDefInfo(RegDef *reg_def);
  IRegister *FindVersionedReg(RegDef *reg_def);

  ITable *table_;
  DebugAnnotation *annotation_;
  IResource *phi_;
  unique_ptr<BBSet> bset_;
  unique_ptr<DataFlow> data_flow_;
  vector<PHI *> phis_;
  map<IInsn *, set<RegDef *>> reg_def_map_;
  // For versioning.
  map<IRegister *, PerRegister *> reg_info_;
  map<RegDef *, int> def_versions_;
};

}  // namespace ssa
}  // namespace opt
}  // namespace iroha

#endif  // _opt_ssa_phi_builder_h_
