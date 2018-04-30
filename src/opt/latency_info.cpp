#include "opt/latency_info.h"

namespace iroha {
namespace opt {

int LatencyInfo::GetInsnLatency(IInsn *insn) {
  return 0;
}

int LatencyInfo::GetRegisterSlack(IState *st, IRegister *reg) {
  return 1;
}

}  // namespace opt
}  // namespace iroha
