#include "opt/pipeline/pipeline_pass.h"

#include "iroha/i_design.h"
#include "iroha/resource_params.h"
#include "opt/loop/loop_block.h"
#include "opt/optimizer_log.h"
#include "opt/pipeline/pipeliner.h"
#include "opt/pipeline/stage_scheduler.h"

namespace iroha {
namespace opt {
namespace pipeline {

PipelinePass::~PipelinePass() {}

Pass *PipelinePass::Create() { return new PipelinePass(); }

bool PipelinePass::ApplyForTable(const string &key, ITable *table) {
  bool is_exp = false;
  if (name_ == "pipeline_x") {
    is_exp = true;
  }
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
    if (params->GetExperimental() && !is_exp) {
      continue;
    }
    loop_regs.push_back(reg);
  }
  for (IRegister *reg : loop_regs) {
    loop::LoopBlock lb(table, reg);
    if (!lb.Build()) {
      continue;
    }
    if (!CheckWriteConflict(&lb)) {
      continue;
    }
    StageScheduler ssch(&lb);
    if (!ssch.Build()) {
      continue;
    }
    ++n;
    Pipeliner pipeliner(table, &ssch);
    pipeliner.Pipeline();
  }
  ostream &os = opt_log_->GetDumpStream();
  os << "Applied " << n << " pipelining<br/>\n";
  return true;
}

bool PipelinePass::CheckWriteConflict(loop::LoopBlock *lb) {
  set<IRegister *> written_regs;
  auto &sts = lb->GetStates();
  for (IState *st : sts) {
    for (IInsn *insn : st->insns_) {
      for (IRegister *oreg : insn->outputs_) {
        if (written_regs.find(oreg) != written_regs.end()) {
          return false;
        }
        written_regs.insert(oreg);
      }
    }
  }
  return true;
}

}  // namespace pipeline
}  // namespace opt
}  // namespace iroha
