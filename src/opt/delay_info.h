// -*- C++ -*-
#ifndef _opt_delay_info_h_
#define _opt_delay_info_h_

#include "opt/common.h"

namespace iroha {
namespace opt {

class DelayInfo {
public:
  DelayInfo(int maxDelay);
  ~DelayInfo();

  // Each caller has to delete it.
  static DelayInfo *Create(int maxDelay);

  int GetInsnDelay(IInsn *insn);
  // For wire/simple_shrink.cpp. This will be deleted.
  int GetInsnLatency(IInsn *insn);
  int GetRegisterSlack(IState *st, IRegister *reg);

private:
  int maxDelay_;
};

}  // namespace opt
}  // namespace iroha

#endif  // _opt_delay_info_h_
