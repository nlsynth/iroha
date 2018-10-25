#include "opt/delay_info.h"

#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "platform/platform_db.h"

namespace iroha {
namespace opt {

DelayInfo::DelayInfo(platform::PlatformDB *platform_db, int max_delay)
  : platform_db_(platform_db), max_delay_(max_delay) {
}

DelayInfo::~DelayInfo() {
}

DelayInfo *DelayInfo::Create(platform::PlatformDB *platform_db, int max_delay) {
  return new DelayInfo(platform_db, max_delay);
}

int DelayInfo::GetMaxDelay() {
  return max_delay_;
}

int DelayInfo::GetInsnDelay(IInsn *insn) {
  IResource *res = insn->GetResource();
  if (resource::IsExtCombinational(*(res->GetClass()))) {
    return max_delay_ - 2;
  }
  int d = platform_db_->GetResourceDelay(res);
  if (d > 0) {
    return d;
  }
  return 1;
}

}  // namespace opt
}  // namespace iroha
