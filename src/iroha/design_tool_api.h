// -*- C++ -*-
#ifndef _iroha_design_tool_api_h_
#define _iroha_design_tool_api_h_

#include "iroha/common.h"
#include "iroha/i_design.h"

namespace iroha {

class DesignToolAPI {
public:
  virtual IDesign *GetDesign() = 0;
  // If table == nullptr, process all tables in current design.
  virtual void ValidateStateId(ITable *table) = 0;
};

}  // namespace iroha

#endif  // _iroha_design_tool_api_h_
