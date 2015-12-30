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
  return (name == kAdd || name == kSub);
}

bool IsNumToBoolBinOp(const IResourceClass &rc) {
  return (rc.GetName() == kGt);
}

bool IsArray(const IResourceClass &rc) {
  return (rc.GetName() == kArray);
}

}  // namespace resource
}  // namespace iroha
