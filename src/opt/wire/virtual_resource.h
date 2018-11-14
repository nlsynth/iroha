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

  ResourceEntry *GetResourceEntry();
  void SetResourceEntry(ResourceEntry *re);
  IInsn *GetInsn();

private:
  VirtualResourceSet *vrset_;
  IInsn *insn_;
  ResourceEntry *res_;
};

// Per table. Managed by DataPathSet.
class VirtualResourceSet {
public:
  VirtualResourceSet(ITable *tab);
  ~VirtualResourceSet();

  VirtualResource *GetFromInsn(IInsn *insn);
  void BuildDefaultBinding();

private:
  ITable *tab_;
  map<IInsn *, VirtualResource *> raw_resources_;
  map<IResource *, ResourceEntry *> default_resource_entries_;
};

}  // namespace wire
}  // namespace opt
}  // namespace iroha

#endif  // _opt_wire_virtual_resource_h_
