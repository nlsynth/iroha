// -*- C++ -*-
#ifndef _opt_wire_resource_share_h_
#define _opt_wire_resource_share_h_

#include "opt/wire/common.h"

namespace iroha {
namespace opt {
namespace wire {

class ResourceEntry;
class BBEntry;

// Manages resource usages in a table.
class ResourceShare {
public:
  ResourceShare(ITable *tab);
  ~ResourceShare();

  void Scan(BBSet *bbs);
  void Allocate();
  void ReBind();

private:
  ITable *tab_;

  map<IResource *, ResourceEntry *> entries_;
  map<BB *, BBEntry *> bb_entries_;
  map<IInsn *, int> rebind_index_;

  void CollectCongestedResource();
  void AssignResourceForOneInsn(IInsn *insn, ResourceEntry *re);
  IInsn *RewriteInsnWithNewResource(IInsn *insn, IResource *res);
};

}  // namespace wire
}  // namespace opt
}  // namespace iroha

#endif  // _opt_wire_resource_share_h_

