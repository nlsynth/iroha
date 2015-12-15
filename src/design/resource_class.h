// -*- C++ -*-
#ifndef _design_resource_class_h_
#define _design_resource_class_h_

namespace iroha {

namespace resource {
const char kSet[] = "set";
const char kTransition[] = "tr";
}  // namespace resource

class IDesign;
class IResourceClass;

void InstallResourceClasses(IDesign *design);
IResourceClass *GetTransitionResourceClassFromDesign(IDesign *design);

}  // namespace iroha

#endif  // _iroha_resource_class_h_
