// -*- C++ -*-
#ifndef _design_design_tool_h_
#define _design_design_tool_h_

#include "iroha/i_design.h"

namespace iroha {

class DesignTool {
public:
  explicit DesignTool(IDesign *design);

  IDesign *GetDesign();
  void Validate(ITable *table);
  static IInsn *AddNextState(IState *cur, IState *next);
  static IState *InsertNextState(IState *st);
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

private:
  IDesign *design_;
};

}  // namespace iroha

#endif  // _design_design_tool_h_
