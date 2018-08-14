// -*- C++ -*-
#ifndef _opt_wire_wire_insn_h_
#define _opt_wire_wire_insn_h_

#include "opt/wire/scaffold.h"

namespace iroha {
namespace opt {
namespace wire {

class WireInsnPhase : public Phase {
public:
  virtual ~WireInsnPhase();

  static Phase *Create();

private:
  virtual bool ApplyForTable(const string &key, ITable *table);
};

class WireInsn : public Scaffold {
public:
  WireInsn(ITable *table, DebugAnnotation *annotation);
  virtual ~WireInsn();
  bool Perform();

private:
protected:
  struct MoveStrategy {
    IInsn *insn;
    bool use_same_resource;
  };
private:
  // Build info.
  void BuildDependency(BB *bb);
  void BuildRWDependencyPair(IInsn *insn, IRegister *source_reg,
                             map<IRegister *, IInsn *> &dep_map);
  // Mutate.
  //  Rewrites from
  //    oreg_0, 1,, n <- insn()
  //  to
  //    owire_0, 1,, n <- insn()
  void ReplaceInsnOutputWithWireBB(BB *bb);
  void ReplaceInsnOutputWithWire(IInsn *insn);
  bool IsSimpleAssign(IInsn *insn);
  void AddWireToRegMapping(IInsn *insn, IRegister *wire, IRegister *reg);
  void ScanBBToMoveInsn(BB *bb);
  int TryToMoveInsnsToTarget(BB *bb, int target_pos);
  void TryMoveInsns(vector<MoveStrategy> &movable_insns, BB *bb,
		    int target_pos, bool use_same_resource);
  bool IsSimpleState(IState *st);
  void MoveLastTransitionInsn(BB *bb);
  bool CanMoveInsn(IInsn *insn, BB *bb, int target_pos, MoveStrategy *ms);
  bool CheckLatency(IInsn *insn, IState *target_st);
  void MoveInsn(MoveStrategy *ms, BB *bb, int target_pos);
  void AddWireToRegisterAssignments();
  bool IsUsedLaterInThisBB(IInsn *insn, IRegister *output);
  // virtual methods to allow different strategies in child classes.
protected:
  virtual bool CanUseResourceInState(IState *st, IResource *resource,
				     MoveStrategy *ms);
private:
  virtual IInsn *MayCopyInsnForState(IState *st, IInsn *insn,
				     MoveStrategy *ms);

};
  
}  // namespace wire
}  // namespace opt
}  // namespace iroha

#endif  // _opt_wire_wire_insn_h_
