// -*- C++ -*-
#ifndef _opt_sched_virtual_resource_set_h_
#define _opt_sched_virtual_resource_set_h_

#include "opt/sched/common.h"

namespace iroha {
namespace opt {
namespace sched {

// Per table. Managed by DataPathSet.
class VirtualResourceSet {
public:
  VirtualResourceSet(ITable *tab);
  ~VirtualResourceSet();

  VirtualResource *GetFromInsn(IInsn *insn);
  void BuildDefaultBinding();
  void PrepareReplicas();
  map<IResource *, ResourceEntry *> &GetResourceEntries();
  map<IInsn *, VirtualResource *> &GetVirtualResources();

private:
  ITable *tab_;
  map<IInsn *, VirtualResource *> raw_resources_;
  map<IResource *, ResourceEntry *> default_resource_entries_;
};

}  // namespace sched
}  // namespace opt
}  // namespace iroha

#endif  // _opt_sched_virtual_resource_set_h_
