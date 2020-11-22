#include "opt/ssa/condition_value_range.h"

namespace iroha {
namespace opt {
namespace ssa {

ConditionValueRange::ConditionValueRange(ITable *table) : table_(table) {}

void ConditionValueRange::Build() {}

ConditionResult ConditionValueRange::Query(const vector<IRegister *> &regs) {
  ConditionResult res;
  res.cond_reg = nullptr;
  return res;
}

}  // namespace ssa
}  // namespace opt
}  // namespace iroha