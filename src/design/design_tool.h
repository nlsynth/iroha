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

private:
  IDesign *design_;
};

}  // namespace iroha

#endif  // _design_design_tool_h_
