#include "opt/sched/wire_phase.h"

#include "iroha/i_design.h"
#include "iroha/resource_params.h"
#include "opt/delay_info.h"
#include "opt/optimizer.h"
#include "opt/profile/profile.h"
#include "opt/sched/wire.h"

namespace iroha {
namespace opt {
namespace sched {

WirePhase::~WirePhase() {
}

Phase *WirePhase::Create() {
  return new WirePhase();
}

bool WirePhase::ApplyForDesign(IDesign *design) {
  if (!profile::Profile::HasProfile(design)) {
    profile::Profile::FillFakeProfile(design);
  }
  profile::Profile::NormalizeProfile(design);
  int max_delay = design->GetParams()->GetMaxDelayPs();
  delay_info_.reset(DelayInfo::Create(optimizer_->GetPlatformDB(), max_delay));
  return Phase::ApplyForDesign(design);
}

bool WirePhase::ApplyForTable(const string &key, ITable *table) {
  Wire wire(table, delay_info_.get(), annotation_);
  return wire.Perform();
}

}  // namespace sched
}  // namespace opt
}  // namespace iroha
