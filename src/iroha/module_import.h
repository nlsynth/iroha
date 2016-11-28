// -*- C++ -*-
#ifndef _iroha_module_import_h_
#define _iroha_module_import_h_

#include "iroha/common.h"

namespace iroha {

class ModuleImportTap {
public:
  ModuleImportTap();

  string source;
  string tag;
  string resource;
  int module_id;
  int table_id;
  int resource_id;
};

// Managed by IModule's unique_ptr.
class ModuleImport {
public:
  ModuleImport(IModule *mod, const string &fn);
  ~ModuleImport();

  const string &GetFileName();

  vector<ModuleImportTap> taps_;

private:
  IModule *mod_;
  string fn_;
};

}  // namespace iroha

#endif  // _iroha_module_import_h_

