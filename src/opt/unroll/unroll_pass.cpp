#include "opt/unroll/unroll_pass.h"

#include "iroha/i_design.h"
#include "iroha/resource_params.h"
#include "opt/loop/loop_block.h"
#include "opt/optimizer_log.h"
#include "opt/unroll/unroller.h"

namespace iroha {
namespace opt {
namespace unroll {

UnrollPass::~UnrollPass() {}

Pass *UnrollPass::Create() { return new UnrollPass(); }

bool UnrollPass::ApplyForTable(const string &key, ITable *table) {
  int n = 0;
  vector<IRegister *> loop_regs;
  for (IRegister *reg : table->registers_) {
    auto *params = reg->GetMutableParams(false);
    if (params == nullptr) {
      continue;
    }
    int unroll_count = params->GetLoopUnroll();
    if (unroll_count <= 1) {
      // 1 for no unroll. 0 for auto (TBD).
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
    auto *params = reg->GetMutableParams(false);
    int unroll_count = params->GetLoopUnroll();
    Unroller unroller(table, &lb, unroll_count);
    unroller.Unroll();
  }
  ostream &os = opt_log_->GetDumpStream();
  os << "Applied " << n << " unrolls<br/>\n";
  return true;
}

}  // namespace unroll
}  // namespace opt
}  // namespace iroha
