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

private:
  ITable *tab_;

  map<IResource *, ResourceEntry *> entries_;
  map<BB *, BBEntry *> bb_entries_;

  void CollectCongestedResource();
};

}  // namespace wire
}  // namespace opt
}  // namespace iroha

#endif  // _opt_wire_resource_share_h_

