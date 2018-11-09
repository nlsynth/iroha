// -*- C++ -*-
#ifndef _opt_wire_virtual_resource_h
#define _opt_wire_virtual_resource_h

#include "opt/wire/common.h"

namespace iroha {
namespace opt {
namespace wire {

class VirtualResource {
public:
  VirtualResource(VirtualResourceSet *vrset, IResource *res);

private:
  VirtualResourceSet *vrset_;
  IResource *res_;
};

// WIP.
// Per table. Managed by DataPathSet.
class VirtualResourceSet {
public:
  VirtualResourceSet(ITable *tab);
  ~VirtualResourceSet();

  VirtualResource *GetOriginalResource(IResource *res);

private:
  ITable *tab_;
  map<IResource *, VirtualResource *> raw_resources_;
};

}  // namespace wire
}  // namespace opt
}  // namespace iroha

#endif  // _opt_wire_virtual_resource_h
