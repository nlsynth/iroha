// -*- C++ -*-
#ifndef _opt_wire_virtual_resource_h
#define _opt_wire_virtual_resource_h

#include "opt/wire/common.h"

namespace iroha {
namespace opt {
namespace wire {

// This takes an insn and uses the information of its original resource until
// everything will be done.
class VirtualResource {
public:
  VirtualResource(VirtualResourceSet *vrset, IInsn *insn);

private:
  VirtualResourceSet *vrset_;
  IInsn *insn_;
};

// Per table. Managed by DataPathSet.
class VirtualResourceSet {
public:
  VirtualResourceSet(ITable *tab);
  ~VirtualResourceSet();

  VirtualResource *GetFromInsn(IInsn *insn);

private:
  ITable *tab_;
  map<IInsn *, VirtualResource *> raw_resources_;
};

}  // namespace wire
}  // namespace opt
}  // namespace iroha

#endif  // _opt_wire_virtual_resource_h
