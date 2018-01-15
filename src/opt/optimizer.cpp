#include "opt/optimizer.h"

#include "design/validator.h"
#include "iroha/i_design.h"
#include "iroha/logging.h"
#include "opt/array_to_mem.h"
#include "opt/clean/empty_state.h"
#include "opt/clean/unreachable_state.h"
#include "opt/clean/pseudo_resource.h"
#include "opt/clean/unused_resource.h"
#include "opt/debug_annotation.h"
#include "opt/phase.h"
#include "opt/ssa/ssa.h"
#include "opt/wire/wire_insn.h"
#include "writer/writer.h"

#include <fstream>

namespace iroha {
namespace opt {

map<string, function<Phase *()> > Optimizer::phases_;

Optimizer::Optimizer(IDesign *design) : design_(design) {
}

void Optimizer::Init() {
  RegisterPhase("array_to_mem", &ArrayToMem::Create);
  RegisterPhase("clean_empty_state", &clean::CleanEmptyStatePhase::Create);
  RegisterPhase("clean_unreachable_state",
		&clean::CleanUnreachableStatePhase::Create);
  RegisterPhase("clean_unused_resource",
		&clean::CleanUnusedResourcePhase::Create);
  RegisterPhase("clean_pseudo_resource",
		&clean::CleanPseudoResourcePhase::Create);
  RegisterPhase("ssa_convert", &ssa::SSAConverterPhase::Create);
  RegisterPhase("phi_cleaner", &ssa::PhiCleanerPhase::Create);
  RegisterPhase("wire_insn", &wire::WireInsnPhase::Create);
}

void Optimizer::RegisterPhase(const string &name,
			      function<Phase *()> factory) {
  phases_[name] = factory;
}

bool Optimizer::ApplyPhase(const string &name) {
  auto it = phases_.find(name);
  if (it == phases_.end()) {
    LOG(USER) << "Unknown optimization phase: " << name;
    return false;
  }
  auto factory = it->second;
  unique_ptr<Phase> phase(factory());
  phase->SetAnnotation(design_->GetDebugAnnotation());
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
  writer::Writer::WriteDumpHeader(os);
  an->GetDumpedContent(os);
  writer::Writer::WriteDumpFooter(os);
}

}  // namespace opt
}  // namespace iroha
