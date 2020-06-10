// -*- C++ -*-
#ifndef _opt_sched_relocator_h_
#define _opt_sched_relocator_h_

#include "opt/sched/common.h"

namespace iroha {
namespace opt {
namespace sched {

// Relocates insns by schedling decisions.
class Relocator {
public:
  Relocator(DataPathSet *data_path_set);

  void Relocate();

private:
  void RelocateInsnsForDataPath(BBDataPath *dp);
  void RewirePaths(BBDataPath *dp, vector<IState *> *states);
  void RewirePathsNode(PathNode *src_node, vector<IState *> *states);
  void RelocateInsn(PathNode *node, vector<IState *> *states);
  void RelocateTransitionInsns(BBDataPath *dp, vector<IState *> *states);
  IRegister *AllocIntermediateReg(IInsn *insn, bool state_local, int oindex);
  IRegister *AddIntermediateWireAndInsn(PathEdge *edge, IState *st);
  IRegister *AddIntermediateRegAndInsn(PathEdge *edge, IState *st);
  void RewriteSinkInput(PathEdge *edge, IRegister *src, IRegister *dst);

  DataPathSet *data_path_set_;
  IResource *assign_;
};

}  // namespace sched
}  // namespace opt
}  // namespace iroha

#endif  // _opt_sched_relocator_h_
