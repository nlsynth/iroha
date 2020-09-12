#include "opt/sched/sched_pass.h"

#include "iroha/i_design.h"
#include "iroha/resource_params.h"
#include "opt/delay_info.h"
#include "opt/optimizer.h"
#include "opt/profile/profile.h"
#include "opt/sched/table_scheduler.h"

namespace iroha {
namespace opt {
namespace sched {

SchedPass::~SchedPass() {}

Pass *SchedPass::Create() { return new SchedPass(); }

bool SchedPass::ApplyForDesign(IDesign *design) {
  if (!profile::Profile::HasProfile(design)) {
    profile::Profile::FillFakeProfile(design);
  }
  profile::Profile::NormalizeProfile(design);
  int max_delay = design->GetParams()->GetMaxDelayPs();
  delay_info_.reset(DelayInfo::Create(optimizer_->GetPlatformDB(), max_delay));
  return Pass::ApplyForDesign(design);
}

bool SchedPass::ApplyForTable(const string &key, ITable *table) {
  TableScheduler sched(table, delay_info_.get(), opt_log_);
  return sched.Perform();
}

}  // namespace sched
}  // namespace opt
}  // namespace iroha
