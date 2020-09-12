#include "opt/optimizer.h"

#include "design/validator.h"
#include "iroha/i_design.h"
#include "iroha/logging.h"
#include "opt/array_elimination.h"
#include "opt/array_split_rdata.h"
#include "opt/clean/empty_state.h"
#include "opt/clean/empty_table.h"
#include "opt/clean/pseudo_resource.h"
#include "opt/clean/unreachable_state.h"
#include "opt/clean/unused_register.h"
#include "opt/clean/unused_resource.h"
#include "opt/compound.h"
#include "opt/constant/constant_propagation.h"
#include "opt/debug_annotation.h"
#include "opt/pass.h"
#include "opt/pipeline/pipeline_phase.h"
#include "opt/sched/sched_phase.h"
#include "opt/ssa/ssa.h"
#include "opt/study.h"
#include "opt/unroll/unroll_phase.h"
#include "platform/platform.h"
#include "platform/platform_db.h"

namespace iroha {
namespace opt {

map<string, function<Pass *()> > Optimizer::phases_;

Optimizer::Optimizer(IDesign *design) : design_(design) {
  design_->SetDebugAnnotation(new DebugAnnotation);
  platform_db_.reset(platform::Platform::CreatePlatformDB(design));
}

Optimizer::~Optimizer() {}

void Optimizer::Init() {
  RegisterPass("array_elimination", &ArrayElimination::Create);
  RegisterPass("array_split_rdata", &ArraySplitRData::Create);
  RegisterPass("clean_empty_state", &clean::CleanEmptyStatePhase::Create);
  RegisterPass("clean_empty_table", &clean::CleanEmptyTablePhase::Create);
  RegisterPass("clean_unreachable_state",
               &clean::CleanUnreachableStatePhase::Create);
  RegisterPass("clean_unused_register", &clean::CleanUnusedRegPhase::Create);
  RegisterPass("clean_unused_resource",
               &clean::CleanUnusedResourcePhase::Create);
  RegisterPass("clean_pseudo_resource",
               &clean::CleanPseudoResourcePhase::Create);
  RegisterPass("constant_propagation", &constant::ConstantPropagation::Create);
  RegisterPass("ssa_convert", &ssa::SSAConverterPhase::Create);
  RegisterPass("phi_cleaner", &ssa::PhiCleanerPhase::Create);
  RegisterPass("alloc_resource", &sched::SchedPhase::Create);
  RegisterPass("wire_insn", &sched::SchedPhase::Create);
  RegisterPass("pipeline", &pipeline::PipelinePhase::Create);
  RegisterPass("unroll", &unroll::UnrollPhase::Create);
  RegisterPass("study", &Study::Create);
  CompoundPhase::Init();
}

vector<string> Optimizer::GetPhaseNames() {
  vector<string> names;
  for (auto it : phases_) {
    names.push_back(it.first);
  }
  return names;
}

void Optimizer::RegisterPass(const string &name, function<Pass *()> factory) {
  phases_[name] = factory;
}

bool Optimizer::ApplyPhase(const string &name) {
  auto it = phases_.find(name);
  if (it == phases_.end()) {
    LOG(USER) << "Unknown optimization phase: " << name;
    return false;
  }
  auto factory = it->second;
  unique_ptr<Pass> pass(factory());
  pass->SetName(name);
  pass->SetOptimizer(this);
  auto *annotation = design_->GetDebugAnnotation();
  pass->SetAnnotation(annotation);
  annotation->StartPhase(name);
  bool isOk = pass->Apply(design_);
  if (isOk) {
    Validator::Validate(design_);
  }
  return isOk;
}

void Optimizer::EnableDebugAnnotation() {
  DebugAnnotation *an = design_->GetDebugAnnotation();
  an->Enable();
}

platform::PlatformDB *Optimizer::GetPlatformDB() { return platform_db_.get(); }

void Optimizer::DumpIntermediateToFiles(const string &fn) {
  if (fn.empty()) {
    return;
  }
  auto *an = design_->GetDebugAnnotation();
  an->WriteToFiles(fn);
}

}  // namespace opt
}  // namespace iroha
