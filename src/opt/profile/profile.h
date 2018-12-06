// -*- C++ -*-
#ifndef _opt_profile_profile_h_
#define _opt_profile_profile_h_

#include "opt/common.h"

namespace iroha {
namespace opt {
namespace profile {

class Profile {
public:
  static bool HasProfile(IDesign *design);
  static void FillFakeProfile(IDesign *design);
  static void ClearProfile(IDesign *design);
  static void NormalizeProfile(IDesign *design);

private:
  static long GetNormalizedCount(long raw_count, long raw_total);
};

}  // namespace profile
}  // namespace opt
}  // namespace iroha

#endif  // _opt_profile_profile_h_
