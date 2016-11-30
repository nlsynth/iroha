// -*- C++ -*-
#ifndef _design_table_copier_h_
#define _design_table_copier_h_

#include "iroha/i_design.h"

#include <map>

namespace iroha {

class TableCopier {
public:
  TableCopier(ITable *src, IModule *new_parent_mod);

  ITable *Copy();

  static ITable *CopyTable(ITable *src, IModule *new_parent_mod);
  static void CopyResourceParams(ResourceParams *src, ResourceParams *dst);
  static void CopyInsnParams(IInsn *src_insn, IInsn *dst_insn);

private:
  void CopyResource();
  void BuildResourceClassMapping();
  void CopyState();
  void CopyRegister();
  void CopyInsnAll();
  IInsn *CopyInsn(IInsn *src_insn);
  static IArray *CopyArray(IArray *src_array, IResource *new_res);

  IModule *mod_;
  ITable *src_tab_;
  ITable *new_tab_;
  map<IResourceClass *, IResourceClass *> resource_class_map_;
  map<IResource *, IResource *> resource_map_;
  map<IState *, IState *> state_map_;
  map<IRegister *, IRegister *> reg_map_;
};

}  // namespace iroha

#endif  // _design_table_copier_h_
