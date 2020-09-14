#include "opt/pipeline/pipeliner.h"

#include "iroha/i_design.h"
#include "opt/loop/loop_block.h"
#include "opt/optimizer_log.h"

namespace iroha {
namespace opt {
namespace pipeline {

Pipeliner::Pipeliner(ITable *tab, loop::LoopBlock *lb)
    : tab_(tab), lb_(lb), opt_log_(nullptr) {
  opt_log_ = tab->GetModule()->GetDesign()->GetOptimizerLog();
}

bool Pipeliner::Pipeline() {
  lb_->Annotate(opt_log_);
  ostream &os = opt_log_->GetDumpStream();
  os << "Pipeliner " << lb_->GetStates().size() << " states, "
     << lb_->GetLoopCount() << " loop <br/>";
  return true;
}

}  // namespace pipeline
}  // namespace opt
}  // namespace iroha
