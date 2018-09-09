// -*- C++ -*-
#ifndef _opt_wire_simple_shrink_h_
#define _opt_wire_simple_shrink_h_

#include "opt/wire/scaffold.h"

namespace iroha {
namespace opt {
namespace wire {

class SimpleShrinkPhase : public Phase {
public:
  virtual ~SimpleShrinkPhase();

  static Phase *Create();

private:
  virtual bool ApplyForTable(const string &key, ITable *table);
};

class SimpleShrink : public Scaffold {
public:
  SimpleShrink(ITable *table, DebugAnnotation *annotation);
  virtual ~SimpleShrink();
  bool Perform();

private:
protected:
  struct MoveStrategy {
    IInsn *insn;
    bool use_same_resource;
  };
private:
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

#endif  // _opt_wire_simple_shrink_h_
