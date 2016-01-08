// -*- C++ -*-
//
// Utility functions to manipulate IDesign data structures.
//
#ifndef _iroha_design_tool_api_h_
#define _iroha_design_tool_api_h_

#include "iroha/common.h"
#include "iroha/i_design.h"

namespace iroha {

class DesignToolAPI {
public:
  virtual ~DesignToolAPI();
  virtual IDesign *GetDesign() = 0;
  // If table == nullptr, process all tables in current design.
  virtual void Validate(ITable *table) = 0;
  virtual IInsn *AddNextState(IState *cur, IState *next) = 0;
  virtual IResource *GetResource(ITable *table,
				 const string &class_name) = 0;
  virtual IResource *GetBinOpResource(ITable *table,
				      const string &class_name, int width) = 0;
  virtual IResource *CreateShifterResource(ITable *table) = 0;
  virtual IResource *CreateArrayResource(ITable *table,
					 int addres_width,
					 int data_width,
					 bool is_external,
					 bool is_ram) = 0;
  virtual IResource *CreateEmbedResource(ITable *table,
					 const string &mod_name,
					 const string &fn) = 0;
  virtual IResource *CreateSubModuleTaskCallResource(ITable *tab,
						     IModule *mod) = 0;
  virtual IResource *CreateTaskResource(ITable *tab) = 0;
  virtual IRegister *AllocRegister(ITable *table, const string &name,
				   int width) = 0;
  virtual IRegister *AllocConstNum(ITable *table,
				   int width, uint64_t value) = 0;
  virtual void SetRegisterInitialValue(uint64_t value, IRegister *reg) = 0;
  virtual IInsn *CreateShiftInsn(IRegister *reg, bool to_left, int amount) = 0;
};

}  // namespace iroha

#endif  // _iroha_design_tool_api_h_
