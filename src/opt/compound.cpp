#include "opt/compound.h"

#include "opt/optimizer.h"

namespace iroha {
namespace opt {

CompoundPass::~CompoundPass() {}

Pass *CompoundPass::Create() { return new CompoundPass(); }

void CompoundPass::Init() {
  Optimizer::RegisterPass("clean", &CompoundPass::Create);
}

bool CompoundPass::ApplyForDesign(IDesign *design) {
  if (name_ == "clean") {
    return optimizer_->ApplyPass("clean_unused_resource") &&
           optimizer_->ApplyPass("clean_unused_register") &&
           optimizer_->ApplyPass("clean_empty_state") &&
           optimizer_->ApplyPass("clean_unreachable_state") &&
           optimizer_->ApplyPass("clean_empty_table");
  }
  return false;
}

}  // namespace opt
}  // namespace iroha
