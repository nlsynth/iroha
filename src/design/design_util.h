// -*- C++ -*-
#ifndef _design_design_util_h_
#define _design_design_util_h_

#include "iroha/common.h"

namespace iroha {

class DesignUtil {
public:
  static IModule *GetRootModule(const IDesign *design);
  static vector<IModule *> GetChildModules(const IModule *mod);
  static IResourceClass *GetTransitionResourceClassFromDesign(IDesign *design);

  static IResource *FindResourceByClassName(ITable *table, const string &name);
  static IResource *FindAssignResource(ITable *table);
  static IResource *FindTransitionResource(ITable *table);
  static IInsn *FindInsnByResource(IState *state, IResource *res);
  static IResourceClass *FindResourceClass(IDesign *design, const string &name);
  static IResource *CreateResource(ITable *table, const string &name);
  static IInsn *GetTransitionInsn(IState *st);
  static IInsn *FindTaskEntryInsn(ITable *table);
};

}  // namespace iroha

#endif  // _design_design_util_h_
