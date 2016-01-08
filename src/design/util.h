// -*- C++ -*-
#ifndef _design_util_h_
#define _design_util_h_

#include "iroha/common.h"

namespace iroha {

class DesignUtil {
public:
  static IModule *GetRootModule(const IDesign *design);
  static vector<IModule *> GetChildModules(const IModule *mod);
  static IResourceClass *GetTransitionResourceClassFromDesign(IDesign *design);
};

}  // namespace iroha

#endif  // _design_util_h_
