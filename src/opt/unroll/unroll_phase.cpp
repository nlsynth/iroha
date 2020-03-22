#include "opt/unroll/unroll_phase.h"

#include "iroha/i_design.h"
#include "iroha/resource_params.h"
#include "opt/unroll/loop_block.h"

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
    if (params != nullptr && params->GetLoopUnroll() != 1) {
      // WIP.
      LoopBlock lb(table, reg);
      lb.Build();
    }
  }
  return true;
}

}  // namespace unroll
}  // namespace opt
}  // namespace iroha
