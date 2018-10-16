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
  DesignBuilder &design_builder_;
};

}  // namespace builder
}  // namespace iroha

#endif  // _builder_platform_builder_h_

