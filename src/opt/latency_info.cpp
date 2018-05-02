#include "opt/latency_info.h"

#include "iroha/i_design.h"
#include "iroha/resource_class.h"

namespace iroha {
namespace opt {

int LatencyInfo::GetInsnLatency(IInsn *insn) {
  if (resource::IsExtCombinational(*(insn->GetResource()->GetClass()))) {
    return 10;
  }
  return 0;
}

int LatencyInfo::GetRegisterSlack(IState *st, IRegister *reg) {
  return 1;
}

}  // namespace opt
}  // namespace iroha
