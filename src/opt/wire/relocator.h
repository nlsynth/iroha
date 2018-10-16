// -*- C++ -*-
#ifndef _opt_wire_relocator_h_
#define _opt_wire_relocator_h_

#include "opt/wire/common.h"

namespace iroha {
namespace opt {
namespace wire {

class Relocator {
public:
  Relocator(DataPathSet *data_path_set);

  void Relocate();

private:
  void RelocateInsnsForDataPath(DataPath *dp);
  IRegister *AllocIntermediateReg(IInsn *insn, bool state_local, int oindex);
  void AddIntermediateWireAndInsn(PathEdge *edge, IState *st);
  void AddIntermediateRegAndInsn(PathEdge *edge, IState *st);

  DataPathSet *data_path_set_;
  IResource *assign_;
};

}  // namespace wire
}  // namespace opt
}  // namespace iroha

#endif  // _opt_wire_relocator_h_