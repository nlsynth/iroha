// -*- C++ -*-
//
// Methods to manipulate design.
// (on the other hand design_tool doesn't mutate)
//
#ifndef _design_design_tool_h_
#define _design_design_tool_h_

#include "iroha/i_design.h"

namespace iroha {

class DesignTool {
public:
  static void Validate(IDesign *design);
  static IInsn *AddNextState(IState *cur, IState *next);
  static IState *InsertNextState(IState *st);
  static void MoveInsn(IInsn *insn, IState *src_st, IState *dst_st);
  static IResource *GetResource(ITable *table,
				const string &class_name);
  static IResource *GetBinOpResource(ITable *table,
				     const string &class_name,
				     int width);
  static IResource *CreateShifterResource(ITable *table);
  static IResource *CreateArrayResource(ITable *table,
					int addres_width,
					int data_width,
					bool is_external,
					bool is_ram);
  static IResource *CreateEmbedResource(ITable *table,
					const string &mod_name,
					const string &fn);
  static IResource *CreateSubModuleTaskCallResource(ITable *table,
						    IModule *mod);
  static IResource *CreateTaskResource(ITable *table);
  static IRegister *AllocRegister(ITable *table, const string &name,
				  int width);
  static IRegister *AllocConstNum(ITable *table,
				  int width, uint64_t value);
  static void SetRegisterInitialValue(uint64_t value,
				      IRegister *reg);
  static IInsn *CreateShiftInsn(IRegister *reg, bool to_left,
				int amount);
  static void DeleteInsn(IState *st, IInsn *insn);
};

}  // namespace iroha

#endif  // _design_design_tool_h_
