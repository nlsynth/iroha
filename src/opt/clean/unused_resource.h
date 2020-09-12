// -*- C++ -*-
#ifndef _opt_clean_unused_resource_h_
#define _opt_clean_unused_resource_h_

#include "opt/pass.h"

namespace iroha {
namespace opt {
namespace clean {

class CleanUnusedResourcePass : public Pass {
 public:
  virtual ~CleanUnusedResourcePass();

  static Pass *Create();

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
