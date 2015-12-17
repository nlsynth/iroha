// -*- C++ -*-
#ifndef _design_design_tool_h_
#define _design_design_tool_h_

#include "iroha/design_tool_api.h"
#include "iroha/i_design.h"

namespace iroha {

class DesignTool : public DesignToolAPI {
public:
  explicit DesignTool(IDesign *design);

  virtual IDesign *GetDesign() override;
  virtual void ValidateStateId(ITable *table) override;
  virtual void SetNextState(IState *cur, IState *next) override;
  virtual IResource *GetResource(ITable *table,
				 const string &class_name) override;
  virtual IRegister *AllocRegister(ITable *table, const string &name,
				   int width) override;
  virtual IRegister *AllocConstNum(ITable *table, const string &name,
				   int width, uint64_t value) override;

private:
  IResource *FindResourceByName(ITable *table, const string &name);
  IResourceClass *FindResourceClass(IDesign *design, const string &name);

  IDesign *design_;
};

}  // namespace iroha

#endif  // _design_design_tool_h_
