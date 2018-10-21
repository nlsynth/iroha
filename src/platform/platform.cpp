#include "platform/platform.h"

#include "builder/design_builder.h"
#include "iroha/i_design.h"
#include "iroha/i_platform.h"
#include "iroha/resource_params.h"

namespace iroha {
namespace platform {

void Platform::ResolvePlatform(IDesign *design) {
  if (design->modules_.size() == 0) {
    return;
  }
  if (design->platforms_.size() > 0) {
    return;
  }
  IPlatform *platform = ReadPlatform(design);
  design->ManagePlatform(platform);
  design->platforms_.push_back(platform);
}

IPlatform *Platform::ReadPlatform(IDesign *design) {
  string platform_name = design->GetParams()->GetPlatformFamily();
  std::unique_ptr<IDesign>
    w(builder::DesignBuilder::ReadDesign(platform_name + ".iroha", true));
  IPlatform *p = nullptr;
  if (w.get() != nullptr && w->platforms_.size() > 0) {
    p = w->platforms_[0];
    w->ReleasePlatform(p);
  }
  if (p == nullptr) {
    p = new IPlatform(nullptr);
  }
  return p;
}

}  // namespace platform
}  // namespace iroha
