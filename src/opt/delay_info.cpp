#include "opt/delay_info.h"

#include "iroha/i_design.h"
#include "iroha/resource_class.h"

namespace iroha {
namespace opt {

int DelayInfo::GetInsnLatency(IInsn *insn) {
  if (resource::IsExtCombinational(*(insn->GetResource()->GetClass()))) {
    return 10;
  }
  return 0;
}

int DelayInfo::GetRegisterSlack(IState *st, IRegister *reg) {
  return 1;
}

}  // namespace opt
}  // namespace iroha
