// -*- C++ -*-
#ifndef _opt_wire_insn_h_
#define _opt_wire_insn_h_

#include "opt/phase.h"

namespace iroha {
namespace opt {

class WireInsnPhase : public Phase {
public:
  virtual ~WireInsnPhase();

  static Phase *Create();

private:
  virtual bool ApplyForTable(ITable *table);
};

class WireInsn {
public:
  WireInsn(ITable *table, DebugAnnotation *annotation);
  ~WireInsn();
  bool Perform();

private:
  class PerInsn {
  public:
    // source of each input. non existent, if it comes from other bb.
    map<IRegister *, IInsn *> depending_insn_;
    // users of each output.
    map<IRegister *, set<IInsn *> > using_insns_;
    int nth_state;
    set<IRegister *> output_reach_to_other_bb_;
  };
  PerInsn *GetPerInsn(IInsn *insn);
  // Build info.
  void CollectReachingRegisters();
  void CollectUsedRegs();
  void BuildDependency(BB *bb);
  void BuildRWDependencyPair(IInsn *insn, IRegister *reg,
                             map<IRegister *, IInsn *> &dep_map);

  ITable *table_;
  DebugAnnotation *annotation_;
  BBSet *bset_;
  DataFlow *data_flow_;
  map<BB *, set<IRegister *>> used_regs_;
  map<IInsn *, PerInsn *> per_insn_map_;
};
  
}  // namespace opt
}  // namespace iroha

#endif  // _opt_wire_insn_h_
