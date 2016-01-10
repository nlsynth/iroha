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
  IInsn *AddNextState(IState *cur, IState *next);
  IResource *GetResource(ITable *table,
			 const string &class_name);
  IResource *GetBinOpResource(ITable *table,
			      const string &class_name,
			      int width);
  IResource *CreateShifterResource(ITable *table);
  IResource *CreateArrayResource(ITable *table,
				 int addres_width,
				 int data_width,
				 bool is_external,
				 bool is_ram);
  IResource *CreateEmbedResource(ITable *table,
				 const string &mod_name,
				 const string &fn);
  IResource *CreateSubModuleTaskCallResource(ITable *table,
					     IModule *mod);
  IResource *CreateTaskResource(ITable *table);
  IRegister *AllocRegister(ITable *table, const string &name,
			   int width);
  IRegister *AllocConstNum(ITable *table,
			   int width, uint64_t value);
  void SetRegisterInitialValue(uint64_t value,
			       IRegister *reg);
  IInsn *CreateShiftInsn(IRegister *reg, bool to_left,
			 int amount);

private:
  IDesign *design_;
};

}  // namespace iroha

#endif  // _design_design_tool_h_
