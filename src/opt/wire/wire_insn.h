// -*- C++ -*-
#ifndef _opt_wire_wire_insn_h_
#define _opt_wire_wire_insn_h_

#include "opt/phase.h"

namespace iroha {
namespace opt {
namespace wire {

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
    PerInsn() : is_assign(false) {
    }
    bool is_assign;
    // Adds assignment instruction (normal <- wire), if the result is
    // used later or in other BBs.
    // {insn, wire register} -> normal register.
    map<IRegister *, IRegister *> wire_to_register_;
    // reverse mapping of above to rewrite inputs.
    map<IRegister *, IRegister *> register_to_wire_;
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
  // Mutate.
  void SplitInsnOutputBB(BB *bb);
  void SplitInsnOutput(IInsn *insn);
  void AddWireToRegMapping(IInsn *insn, IRegister *wire, IRegister *reg);
  void ScanBBToMoveInsn(BB *bb);
  bool CanMoveInsn(IInsn *insn, BB *bb, int target_pos);
  void MoveInsn(IInsn *insn, BB *bb, int target_pos);
  bool CanUseResourceInState(IState *st, IResource *resource);
  void AddWireToRegisterAssignments();
  bool IsUsedLaterInThisBB(IInsn *insn, IRegister *output);

  ITable *table_;
  IResource *assign_;
  IResource *transition_;
  DebugAnnotation *annotation_;
  BBSet *bset_;
  DataFlow *data_flow_;
  map<BB *, set<IRegister *> > used_regs_;
  map<IInsn *, PerInsn *> per_insn_map_;
};
  
}  // namespace wire
}  // namespace opt
}  // namespace iroha

#endif  // _opt_wire_wire_insn_h_
