// -*- C++ -*-
//
// Main APIs and structures are defined (or included from this file).
//
#ifndef _iroha_iroha_h_
#define _iroha_iroha_h_

#include "design/design_tool.h"
#include "iroha/common.h"
#include "iroha/i_design.h"
#include "iroha/opt_api.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"
#include "iroha/writer_api.h"

namespace iroha {

// High level APIs and factory methods.
class Iroha {
public:
  static void Init();
  static IDesign *ReadDesignFromFile(const string &fn);
  static WriterAPI *CreateWriter(const IDesign *design);
  // Set design nullptr to create a new design.
  static DesignTool *CreateDesignTool(IDesign *design);
  static OptAPI *CreateOptimizer(IDesign *design);
};

}  // namespace iroha

#endif  // _iroha_iroha_h_
