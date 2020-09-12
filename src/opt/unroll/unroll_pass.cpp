#include "opt/unroll/unroll_pass.h"

#include "iroha/i_design.h"
#include "iroha/resource_params.h"
#include "opt/loop/loop_block.h"
#include "opt/unroll/unroller.h"

namespace iroha {
namespace opt {
namespace unroll {

UnrollPass::~UnrollPass() {}

Pass *UnrollPass::Create() { return new UnrollPass(); }

bool UnrollPass::ApplyForTable(const string &key, ITable *table) {
  for (IRegister *reg : table->registers_) {
    auto *params = reg->GetParams(false);
    if (params == nullptr) {
      continue;
    }
    int unroll_count = params->GetLoopUnroll();
    if (unroll_count <= 1) {
      // 1 for no unroll. 0 for auto (TBD).
      continue;
    }
    loop::LoopBlock lb(table, reg);
    if (!lb.Build()) {
      continue;
    }
    Unroller unroller(table, &lb, unroll_count);
    unroller.Unroll();
  }
  return true;
}

}  // namespace unroll
}  // namespace opt
}  // namespace iroha
