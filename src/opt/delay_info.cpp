#include "opt/delay_info.h"

#include "iroha/i_design.h"
#include "iroha/resource_class.h"

namespace iroha {
namespace opt {

DelayInfo::DelayInfo(int maxDelay) : max_delay_(maxDelay) {
}

DelayInfo::~DelayInfo() {
}

DelayInfo *DelayInfo::Create(int maxDelay) {
  return new DelayInfo(maxDelay);
}

int DelayInfo::GetMaxDelay() {
  return max_delay_;
}

int DelayInfo::GetInsnDelay(IInsn *insn) {
  if (resource::IsExtCombinational(*(insn->GetResource()->GetClass()))) {
    return max_delay_ - 2;
  }
  return 1;
}

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
