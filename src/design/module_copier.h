// -*- C++ -*-
#ifndef _design_module_coper_h_
#define _design_module_coper_h_

#include "iroha/i_design.h"

#include <map>

namespace iroha {

class ModuleCopier {
public:
  ModuleCopier(IModule *src, IModule *empty_mod);

  static void CopyModule(IModule *src, IModule *empty_mod);

  void Copy();

private:
  void CopyRec(IModule *src, IModule *dst);
  int UnusedId();

  IModule *src_root_mod_;
  IModule *new_root_mod_;
  map<int, IModule *> mod_id_in_new_design_;
  map<IModule *, IModule *> module_map_;
};

}  // namespace iroha

#endif  // _design_module_copier_h_
