// -*- C++ -*-
#ifndef _design_importer_h_
#define _design_importer_h_

#include "iroha/common.h"

#include <map>

using std::map;

namespace iroha {

class Importer {
public:
  Importer(IDesign *design);

  static void Import(IDesign *design);

  void Resolve();
  
private:
  void TraverseModule(IModule *mod);
  void ProcessImport(IModule *mod);
  void ProcessTapAll();
  void ClearModuleImport();
  void ProcessTap(IModule *mod);
  IResource *RemapResource(const ModuleImportTap &tap, IResource *src_res);
  void ConnectResources(IResource *w, IResource *r);

  IDesign *design_;
  map<string, vector<IResource *> > tag_to_resources_;
};

}  // namespace iroha

#endif  // _design_importer_h_
