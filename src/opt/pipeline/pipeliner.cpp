#include "opt/pipeline/pipeliner.h"

#include "iroha/i_design.h"
#include "opt/loop/loop_block.h"

namespace iroha {
namespace opt {
namespace pipeline {

Pipeliner::Pipeliner(ITable *tab, loop::LoopBlock *lb)
    : tab_(tab), lb_(lb), opt_log_(nullptr) {
  opt_log_ = tab->GetModule()->GetDesign()->GetOptimizerLog();
}

bool Pipeliner::Pipeline() {
  lb_->Annotate(opt_log_);
  return true;
}

}  // namespace pipeline
}  // namespace opt
}  // namespace iroha
