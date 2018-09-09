// -*- C++ -*-
#ifndef _opt_wire_scaffold_h_
#define _opt_wire_scaffold_h_

#include "opt/phase.h"
#include "opt/wire/common.h"

namespace iroha {
namespace opt {
namespace wire {

// Holds data structures to perform the optimization.
// Actual optimization and the strategy will be implemented in child classes.
// TODO: Move more code from WireInsn.
class Scaffold {
public:
  Scaffold(ITable *table, DebugAnnotation *annotation);
  virtual ~Scaffold();

protected:
  void SetUp();
  
  class PerInsn {
  public:
    PerInsn() : is_simple_assign(false) {
    }
    bool is_simple_assign;
    // Adds assignment instruction (normal <- wire), if the result is
    // used later or in other BBs.
    // {insn, wire register} -> normal register.
    map<IRegister *, IRegister *> wire_to_register_;
    // reverse mapping of above to rewrite inputs.
    map<IRegister *, IRegister *> register_to_wire_;
    // insn dependency for each input to keep ordering.
    // non existent if the register comes from other bb.
    map<IRegister *, IInsn *> depending_insn_;
    // users of each output (reverse mapping of depending_insn_).
    map<IRegister *, set<IInsn *> > using_insns_;
    // state index in its bb.
    int nth_state;
    set<IRegister *> output_reach_to_other_bb_;
  };

  PerInsn *GetPerInsn(IInsn *insn);

  ITable *table_;
  unique_ptr<BBSet> bset_;
  unique_ptr<DataFlow> data_flow_;
  IResource *assign_;
  IResource *transition_;
  DebugAnnotation *annotation_;
  map<IInsn *, PerInsn *> per_insn_map_;
  map<BB *, set<IRegister *> > used_regs_;

  void CollectReachingRegisters();
  void CollectUsedRegsPerBB();
  void BuildDependency(BB *bb);
  void BuildRWDependencyPair(IInsn *insn, IRegister *source_reg,
                             map<IRegister *, IInsn *> &last_rw_insn_for_reg);
};

}  // namespace wire
}  // namespace opt
}  // namespace iroha

#endif  // _opt_wire_scaffold_h_
