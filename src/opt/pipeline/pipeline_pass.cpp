#include "opt/pipeline/pipeline_pass.h"

#include "iroha/i_design.h"
#include "iroha/resource_params.h"
#include "opt/loop/loop_block.h"
#include "opt/optimizer_log.h"
#include "opt/pipeline/pipeliner.h"

namespace iroha {
namespace opt {
namespace pipeline {

PipelinePass::~PipelinePass() {}

Pass *PipelinePass::Create() { return new PipelinePass(); }

bool PipelinePass::ApplyForTable(const string &key, ITable *table) {
  int n = 0;
  vector<IRegister *> loop_regs;
  for (IRegister *reg : table->registers_) {
    auto *params = reg->GetParams(false);
    if (params == nullptr) {
      continue;
    }
    if (!params->GetIsPipeline()) {
      continue;
    }
    loop_regs.push_back(reg);
  }
  for (IRegister *reg : loop_regs) {
    loop::LoopBlock lb(table, reg);
    if (!lb.Build()) {
      continue;
    }
    ++n;
    Pipeliner pipeliner(table, &lb);
    pipeliner.Pipeline();
  }
  ostream &os = opt_log_->GetDumpStream();
  os << "Applied " << n << " pipelining<br/>\n";
  return true;
}

}  // namespace pipeline
}  // namespace opt
}  // namespace iroha
