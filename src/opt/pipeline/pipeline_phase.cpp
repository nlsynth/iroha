#include "opt/pipeline/pipeline_phase.h"

namespace iroha {
namespace opt {
namespace pipeline {

PipelinePhase::~PipelinePhase() {
}

Phase *PipelinePhase::Create() {
  return new PipelinePhase();
}

bool PipelinePhase::ApplyForTable(const string &key, ITable *table) {
  return true;
}

}  // namespace pipeline
}  // namespace opt
}  // namespace iroha
