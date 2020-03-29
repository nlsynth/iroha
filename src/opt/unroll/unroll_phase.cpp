#include "opt/unroll/unroll_phase.h"

#include "iroha/i_design.h"
#include "iroha/resource_params.h"
#include "opt/unroll/loop_block.h"
#include "opt/unroll/unroller.h"

namespace iroha {
namespace opt {
namespace unroll {

UnrollPhase::~UnrollPhase() {
}

Phase *UnrollPhase::Create() {
  return new UnrollPhase();
}

bool UnrollPhase::ApplyForTable(const string &key, ITable *table) {
  for (IRegister *reg : table->registers_) {
    auto *params = reg->GetParams(false);
    if (params == nullptr) {
      continue;
    }
    int count = params->GetLoopUnroll();
    if (count != 1) {
      LoopBlock lb(table, reg);
      if (!lb.Build()) {
	continue;
      }
      Unroller unroller(table, &lb, count);
      unroller.Unroll();
    }
  }
  return true;
}

}  // namespace unroll
}  // namespace opt
}  // namespace iroha
