#include "iroha/resource_class.h"

#include "iroha/i_design.h"

namespace iroha {
namespace resource {

bool IsBinOp(const IResourceClass &rc) {
  return IsNumToNumBinOp(rc) ||
    IsNumToBoolBinOp(rc);
}

bool IsNumToNumBinOp(const IResourceClass &rc) {
  const string &name = rc.GetName();
  if (name == kAdd || name == kSub) {
    return true;
  }
  return false;
}

bool IsNumToBoolBinOp(const IResourceClass &rc) {
  if (rc.GetName() == kGt) {
    return true;
  }
  return false;
}

}  // namespace resource
}  // namespace iroha
