// -*- C++ -*-
#ifndef _opt_wire_resource_share_h_
#define _opt_wire_resource_share_h_

#include "opt/wire/common.h"

namespace iroha {
namespace opt {
namespace wire {

class ResourceUsageEntry;
class BBEntry;

// Manages resource usages in a table and reallocate congested resources.
// (We might migrate to VirtualResource based implementation)
class ResourceShare {
public:
  ResourceShare(ITable *tab);
  ~ResourceShare();

  void Scan(BBSet *bbs);
  void Allocate();
  void ReBind();

private:
  ITable *tab_;

  map<IResource *, ResourceUsageEntry *> entries_;
  map<BB *, BBEntry *> bb_entries_;
  map<IInsn *, int> rebind_index_;

  void CollectCongestedResource();
  void AssignResourceForOneInsn(IInsn *insn, ResourceUsageEntry *re);
};

}  // namespace wire
}  // namespace opt
}  // namespace iroha

#endif  // _opt_wire_resource_share_h_

