// -*- C++ -*-
#ifndef _iroha_module_import_h_
#define _iroha_module_import_h_

#include "iroha/common.h"

namespace iroha {

class ModuleImportTap {
public:
  ModuleImportTap();

  // pin name of ext-input or ext-output.
  string source;
  // arbitrary name assigned by user.
  string tag;
  // ext-output, ext-input can be shared-reg or shared-reg-reader.
  string resource_class;

  IResource *resource;
};

// Managed by IModule's unique_ptr.
class ModuleImport {
public:
  ModuleImport(IModule *mod, const string &fn);
  ~ModuleImport();

  const string &GetFileName();

  vector<ModuleImportTap *> taps_;

private:
  IModule *mod_;
  string fn_;
};

}  // namespace iroha

#endif  // _iroha_module_import_h_

