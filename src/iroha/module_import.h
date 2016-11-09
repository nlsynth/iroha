// -*- C++ -*-
#ifndef _iroha_module_import_h_
#define _iroha_module_import_h_

#include "iroha/common.h"

namespace iroha {

class ModuleImport {
public:
  ModuleImport(IModule *mod, const string &fn);

  const string &GetFileName();

private:
  IModule *mod_;
  string fn_;
};

}  // namespace iroha

#endif  // _iroha_module_import_h_

