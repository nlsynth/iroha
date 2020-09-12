#include "opt/pipeline/pipeline_pass.h"

namespace iroha {
namespace opt {
namespace pipeline {

PipelinePass::~PipelinePass() {}

Pass *PipelinePass::Create() { return new PipelinePass(); }

bool PipelinePass::ApplyForTable(const string &key, ITable *table) {
  return true;
}

}  // namespace pipeline
}  // namespace opt
}  // namespace iroha
