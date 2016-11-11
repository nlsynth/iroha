// -*- C++ -*-
#ifndef _design_importer_h_
#define _design_importer_h_

#include "iroha/common.h"

namespace iroha {

class Importer {
public:
  Importer(IDesign *design);

  static void Import(IDesign *design);

  void Resolve();
  
private:
  void TraverseModule(IModule *mod);
  void ProcessImport(IModule *mod);

  IDesign *design_;
};

}  // namespace iroha

#endif  // _design_importer_h_
