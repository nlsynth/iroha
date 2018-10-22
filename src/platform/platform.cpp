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
  string platform_family = design->GetParams()->GetPlatformFamily();
  std::unique_ptr<IDesign>
    tmp_design(builder::DesignBuilder::ReadDesign(platform_family + ".iroha", true));
  IPlatform *selected_pl = nullptr;
  if (tmp_design.get() != nullptr && tmp_design->platforms_.size() > 0) {
    string platform_name = design->GetParams()->GetPlatformName();
    for (IPlatform *pl : tmp_design->platforms_) {
      if (pl->GetName() == platform_name) {
	selected_pl = pl;
      }
    }
    // Selects first one for now.
    if (selected_pl == nullptr) {
      selected_pl = tmp_design->platforms_[0];
    }
    tmp_design->ReleasePlatform(selected_pl);
  }
  if (selected_pl == nullptr) {
    selected_pl = new IPlatform(nullptr);
    selected_pl->SetName("empty");
  }
  return selected_pl;
}

}  // namespace platform
}  // namespace iroha
