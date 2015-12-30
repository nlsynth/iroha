// -*- C++ -*-
#ifndef _design_design_tool_h_
#define _design_design_tool_h_

#include "iroha/design_tool_api.h"
#include "iroha/i_design.h"

#include <set>

namespace iroha {

class DesignTool : public DesignToolAPI {
public:
  explicit DesignTool(IDesign *design);

  virtual IDesign *GetDesign() override;
  virtual void ValidateIds(ITable *table) override;
  virtual IInsn *AddNextState(IState *cur, IState *next) override;
  virtual IResource *GetResource(ITable *table,
				 const string &class_name) override;
  virtual IResource *GetBinOpResource(ITable *table,
				      const string &class_name,
				      int width) override;
  virtual IResource *CreateArrayResource(ITable *table,
					 int addres_width,
					 int data_width,
					 bool is_external,
					 bool is_ram) override;
  virtual IResource *CreateEmbedResource(ITable *table,
					 const string &mod_name,
					 const string &fn) override;
  virtual IRegister *AllocRegister(ITable *table, const string &name,
				   int width) override;
  virtual IRegister *AllocConstNum(ITable *table,
				   int width, uint64_t value) override;
  virtual void SetRegisterInitialValue(uint64_t value,
				       IRegister *reg) override;

private:
  IResource *FindResourceByName(ITable *table, const string &name);
  IInsn *FindInsnByResource(IState *state, IResource *res);
  IResourceClass *FindResourceClass(IDesign *design, const string &name);
  void ValidateStateId(ITable *table);
  void ValidateInsnId(ITable *table);
  void ValidateRegisterId(ITable *table);
  void ValidateResourceId(ITable *table);
  void ValidateTableId(IModule *mod);

  IDesign *design_;
};

}  // namespace iroha

#endif  // _design_design_tool_h_
