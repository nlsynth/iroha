// -*- C++ -*-
#ifndef _design_resource_copier_h_
#define _design_resource_copier_h_

#include "iroha/i_design.h"

#include <map>

namespace iroha {

class ResourceCopier {
public:
  ResourceCopier(IModule *root, map<IModule *, IModule *> &module_map);

  void Copy();

private:
  void TraverseModule(IModule *mod);
  void ProcessTable(ITable *tab);
  void ProcessResource(IResource *res);
  IResource *FindResource(IModule *mod, int tab_id, int res_id);
  ITable *FindTable(IModule *mod, int tab_id);
  void SetCalleeTable(ITable *callee_table, IResource *res);

  IModule *new_root_mod_;
  // src to dst.
  map<IModule *, IModule *> &module_map_;
  map<IModule *, IModule *> reverse_map_;
};

}  // namespace iroha

#endif  // _design_resource_copier_h_
