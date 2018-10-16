#include "builder/platform_builder.h"

#include "builder/design_builder.h"
#include "iroha/i_design.h"
#include "iroha/i_platform.h"

namespace iroha {
namespace builder {

PlatformBuilder::PlatformBuilder(DesignBuilder &design_builder)
  : design_builder_(design_builder) {
}

void PlatformBuilder::BuildPlatform(Exp *e, IDesign *design) {
  IPlatform *platform = new IPlatform(design);
  design->platforms_.push_back(platform);
}

}  // namespace builder
}  // namespace iroha
