// -*- C++ -*-
#ifndef _opt_delay_info_h_
#define _opt_delay_info_h_

#include "opt/common.h"

namespace iroha {
namespace opt {

class DelayInfo {
public:
  DelayInfo(platform::PlatformDB *platform_db, int max_delay);
  ~DelayInfo();

  // Each caller has to delete it.
  static DelayInfo *Create(platform::PlatformDB *platform_db, int max_delay);

  int GetMaxDelay();
  int GetInsnDelay(IInsn *insn);
  // For wire/simple_shrink.cpp. This will be deleted.
  int GetInsnLatency(IInsn *insn);
  int GetRegisterSlack(IState *st, IRegister *reg);

private:
  platform::PlatformDB *platform_db_;
  int max_delay_;
};

}  // namespace opt
}  // namespace iroha

#endif  // _opt_delay_info_h_
