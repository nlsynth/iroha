#include "opt/optimizer.h"

#include "design/validator.h"
#include "iroha/i_design.h"
#include "opt/array_to_mem.h"
#include "opt/bb_shrink.h"
#include "opt/debug_annotation.h"
#include "opt/phase.h"
#include "opt/wire_insn.h"

namespace iroha {
namespace opt {

map<string, function<Phase *()>> Optimizer::phases_;

Optimizer::Optimizer(IDesign *design) : design_(design) {
}

void Optimizer::Init() {
  RegisterPhase("array_to_mem", &ArrayToMem::Create);
  RegisterPhase("bb_shrink", &BBShrinkPhase::Create);
  RegisterPhase("wire_insn", &WireInsnPhase::Create);
}

void Optimizer::RegisterPhase(const string &name,
			      function<Phase *()> factory) {
  phases_[name] = factory;
}

bool Optimizer::ApplyPhase(const string &name) {
  auto it = phases_.find(name);
  if (it == phases_.end()) {
    return false;
  }
  auto factory = it->second;
  unique_ptr<Phase> phase(factory());
  if (phase->ApplyForDesign(design_)) {
    Validator::Validate(design_);
    return true;
  }
  return false;
}

void Optimizer::EnableDebugAnnotation() {
  design_->SetDebugAnnotation(new DebugAnnotation);
}

}  // namespace opt
}  // namespace iroha
