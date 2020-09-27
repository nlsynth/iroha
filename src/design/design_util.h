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
  static IResource *FindOneResourceByClassName(ITable *table,
                                               const string &name);
  static IResource *FindAssignResource(ITable *table);
  static IResource *FindTransitionResource(ITable *table);
  static IInsn *FindInsnByResource(IState *state, IResource *res);
  static IResourceClass *FindResourceClass(IDesign *design, const string &name);
  static IResource *CreateResource(ITable *table, const string &name);
  static IInsn *FindTransitionInsn(IState *st);
  static IInsn *GetTransitionInsn(IState *st);
  static IInsn *FindTaskEntryInsn(ITable *table);
  static IInsn *FindDataFlowInInsn(ITable *table);
  static vector<IInsn *> GetInsnsByResource(const IResource *res);
  static bool IsTerminalState(IState *st);
  static IResource *FindResourceById(ITable *tab, int res_id);
  static ITable *FindTableById(IModule *mod, int tab_id);
  static IRegister *FindRegisterById(ITable *tab, int reg_id);

 private:
  static IInsn *FindInitialInsnByClassName(ITable *tab, const string &name);
};

}  // namespace iroha

#endif  // _design_design_util_h_
