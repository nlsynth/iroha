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

  static void FindResourceByClassName(ITable *table, const string &name,
				      vector<IResource *> *resources);
  static IResource *FindOneResourceByClassName(ITable *table, const string &name);
  static IResource *FindAssignResource(ITable *table);
  static IResource *FindTransitionResource(ITable *table);
  static IInsn *FindInsnByResource(IState *state, IResource *res);
  static IResourceClass *FindResourceClass(IDesign *design, const string &name);
  static IResource *CreateResource(ITable *table, const string &name);
  static IInsn *FindTransitionInsn(IState *st);
  static IInsn *GetTransitionInsn(IState *st);
  static IInsn *FindTaskEntryInsn(ITable *table);
  static bool IsTerminalState(IState *st);
  static bool IsMultiCycleInsn(IInsn *insn);
  static int NumMultiCycleInsn(IState *st);
};

}  // namespace iroha

#endif  // _design_design_util_h_
