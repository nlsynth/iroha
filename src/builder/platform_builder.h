// -*- C++ -*-
#ifndef _builder_platform_builder_h_
#define _builder_platform_builder_h_

#include "iroha/common.h"

namespace iroha {
namespace builder {

class DesignBuilder;
class Exp;

class PlatformBuilder {
public:
  PlatformBuilder(DesignBuilder &design_builder);
  void BuildPlatform(Exp *e, IDesign *design);

private:
  void BuildDefinition(Exp *e, IPlatform *platform);
  void BuildCondition(Exp *e, platform::Definition *def);
  void BuildValue(Exp *e, platform::Definition *def);
  platform::DefNode *BuildNode(Exp *e, platform::Definition *def);
  platform::DefNode *BuildConjunctionFrom2nd(Exp *e, platform::Definition *def);

  DesignBuilder &design_builder_;
};

}  // namespace builder
}  // namespace iroha

#endif  // _builder_platform_builder_h_

