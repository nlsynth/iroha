// -*- C++ -*-
#ifndef _opt_delay_info_h_
#define _opt_delay_info_h_

#include "opt/common.h"

namespace iroha {
namespace opt {

class DelayInfo {
public:
  int GetInsnLatency(IInsn *insn);
  int GetRegisterSlack(IState *st, IRegister *reg);
};

}  // namespace opt
}  // namespace iroha

#endif  // _opt_delay_info_h_
