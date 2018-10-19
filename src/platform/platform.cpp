#include "platform/platform.h"

#include "iroha/i_design.h"
#include "iroha/i_platform.h"

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
  return new IPlatform(nullptr);
}

}  // namespace platform
}  // namespace iroha
