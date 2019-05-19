// -*- C++ -*-
#ifndef _opt_clean_unused_resource_h_
#define _opt_clean_unused_resource_h_

#include "opt/phase.h"

namespace iroha {
namespace opt {
namespace clean {

class CleanUnusedResourcePhase : public Phase {
public:
  virtual ~CleanUnusedResourcePhase();

  static Phase *Create();

private:
  virtual bool ApplyForDesign(IDesign *design);
  virtual bool ApplyForTable(const string &key, ITable *table);

  bool ScanTable(ITable *table);
  void MarkResource(IResource *res);
  bool CollectResource(ITable *table);

  set<IResource *> used_resources_;
  set<IArrayImage *> used_images_;
};

}  // namespace clean
}  // namespace opt
}  // namespace iroha

#endif  // _opt_clean_unused_resource_h_

