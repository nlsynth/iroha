#include "opt/optimizer.h"

#include "design/validator.h"
#include "iroha/i_design.h"
#include "opt/array_to_mem.h"
#include "opt/clean/empty_state.h"
#include "opt/clean/unreachable_state.h"
#include "opt/debug_annotation.h"
#include "opt/phase.h"
#include "opt/wire/wire_insn.h"

#include <fstream>

namespace iroha {
namespace opt {

map<string, function<Phase *()>> Optimizer::phases_;

Optimizer::Optimizer(IDesign *design) : design_(design) {
}

void Optimizer::Init() {
  RegisterPhase("array_to_mem", &ArrayToMem::Create);
  RegisterPhase("clean_empty_state", &clean::CleanEmptyStatePhase::Create);
  RegisterPhase("clean_unreachable_state",
		&clean::CleanUnreachableStatePhase::Create);
  RegisterPhase("wire_insn", &wire::WireInsnPhase::Create);
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

void Optimizer::DumpIntermediate(const string &fn) {
  auto *an = design_->GetDebugAnnotation();
  ofstream os(fn);
  an->GetDumpedContent(os);
}

}  // namespace opt
}  // namespace iroha
