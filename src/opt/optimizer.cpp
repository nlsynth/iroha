#include "opt/optimizer.h"

#include "design/validator.h"
#include "opt/array_to_mem.h"
#include "opt/bb_shrink.h"
#include "opt/phase.h"

namespace iroha {
namespace opt {

map<string, function<Phase *()>> Optimizer::phases_;

Optimizer::Optimizer(IDesign *design) : design_(design) {
}

void Optimizer::Init() {
  RegisterPhase("array_to_mem", &ArrayToMem::Create);
  RegisterPhase("bb_shrink", &BBShrink::Create);
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
    Validator::Validate(design_, nullptr);
    return true;
  }
  return false;
}

}  // namespace opt
}  // namespace iroha
