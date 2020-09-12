#include "opt/compound.h"

#include "opt/optimizer.h"

namespace iroha {
namespace opt {

CompoundPhase::~CompoundPhase() {}

Pass *CompoundPhase::Create() { return new CompoundPhase(); }

void CompoundPhase::Init() {
  Optimizer::RegisterPass("clean", &CompoundPhase::Create);
}

bool CompoundPhase::ApplyForDesign(IDesign *design) {
  if (name_ == "clean") {
    return optimizer_->ApplyPhase("clean_unused_resource") &&
           optimizer_->ApplyPhase("clean_unused_register") &&
           optimizer_->ApplyPhase("clean_empty_state") &&
           optimizer_->ApplyPhase("clean_unreachable_state") &&
           optimizer_->ApplyPhase("clean_empty_table");
  }
  return false;
}

}  // namespace opt
}  // namespace iroha
