#include "iroha/resource_class.h"

#include "iroha/i_design.h"

namespace iroha {
namespace resource {

bool IsExclusiveBinOp(const IResourceClass &rc) {
  return IsNumToNumExclusiveBinOp(rc) ||
    IsNumToBoolExclusiveBinOp(rc);
}

bool IsNumToNumExclusiveBinOp(const IResourceClass &rc) {
  const string &name = rc.GetName();
  return (name == kAdd || name == kSub);
}

bool IsNumToBoolExclusiveBinOp(const IResourceClass &rc) {
  return (rc.GetName() == kGt);
}

bool IsLightBinOp(const IResourceClass &rc) {
  return (rc.GetName() == kXor);
}

bool IsBitArrangeOp(const IResourceClass &rc) {
  return (rc.GetName() == kShift);
}

bool IsArray(const IResourceClass &rc) {
  return (rc.GetName() == kArray);
}

bool IsSubModuleTaskCall(const IResourceClass &rc) {
  return (rc.GetName() == kSubModuleTaskCall);
}

bool IsSubModuleTask(const IResourceClass &rc) {
  return (rc.GetName() == kSubModuleTask);
}

}  // namespace resource
}  // namespace iroha
