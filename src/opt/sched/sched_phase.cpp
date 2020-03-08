#include "opt/sched/sched_phase.h"

#include "iroha/i_design.h"
#include "iroha/resource_params.h"
#include "opt/delay_info.h"
#include "opt/optimizer.h"
#include "opt/profile/profile.h"
#include "opt/sched/table_scheduler.h"

namespace iroha {
namespace opt {
namespace sched {

SchedPhase::~SchedPhase() {
}

Phase *SchedPhase::Create() {
  return new SchedPhase();
}

bool SchedPhase::ApplyForDesign(IDesign *design) {
  if (!profile::Profile::HasProfile(design)) {
    profile::Profile::FillFakeProfile(design);
  }
  profile::Profile::NormalizeProfile(design);
  int max_delay = design->GetParams()->GetMaxDelayPs();
  delay_info_.reset(DelayInfo::Create(optimizer_->GetPlatformDB(), max_delay));
  return Phase::ApplyForDesign(design);
}

bool SchedPhase::ApplyForTable(const string &key, ITable *table) {
  TableScheduler sched(table, delay_info_.get(), annotation_);
  return sched.Perform();
}

}  // namespace sched
}  // namespace opt
}  // namespace iroha
