#include "iroha/resource_class.h"

#include "design/object_pool.h"
#include "iroha/i_design.h"
#include "iroha/resource_class.h"

namespace iroha {
namespace resource {

bool IsTransition(const IResourceClass &rc) {
  return (rc.GetName() == kTransition);
}

bool IsSelect(const IResourceClass &rc) {
  return (rc.GetName() == kSelect);
}

bool IsPseudo(const IResourceClass &rc) {
  return (rc.GetName() == kPseudo);
}

bool IsExclusiveBinOp(const IResourceClass &rc) {
  return IsNumToNumExclusiveBinOp(rc) ||
    IsNumToBoolExclusiveBinOp(rc);
}

bool IsNumToNumExclusiveBinOp(const IResourceClass &rc) {
  const string &name = rc.GetName();
  return (name == kAdd || name == kSub || name == kMul);
}

bool IsNumToBoolExclusiveBinOp(const IResourceClass &rc) {
  const string &name = rc.GetName();
  return (name == kGt || name == kGte || name == kEq);
}

bool IsLightBinOp(const IResourceClass &rc) {
  const string &name = rc.GetName();
  return (name == kBitAnd || name == kBitOr || name == kBitXor);
}

bool IsLightUniOp(const IResourceClass &rc) {
  return (rc.GetName() == kBitInv);
}

bool IsBitShiftOp(const IResourceClass &rc) {
  return (rc.GetName() == kShift);
}

bool IsArray(const IResourceClass &rc) {
  return (rc.GetName() == kArray);
}

bool IsSet(const IResourceClass &rc) {
  return (rc.GetName() == kSet);
}

bool IsBitSel(const IResourceClass &rc) {
  return (rc.GetName() == kBitSel);
}

bool IsBitConcat(const IResourceClass &rc) {
  return (rc.GetName() == kBitConcat);
}

bool IsExtInput(const IResourceClass &rc) {
  return (rc.GetName() == kExtInput);
}

bool IsExtOutput(const IResourceClass &rc) {
  return (rc.GetName() == kExtOutput);
}

bool IsSharedRegReader(const IResourceClass &rc) {
  return (rc.GetName() == kSharedRegReader);
}

bool IsSharedReg(const IResourceClass &rc) {
  return (rc.GetName() == kSharedReg);
}

bool IsChannelRead(const IResourceClass &rc) {
  return (rc.GetName() == kChannelRead);
}

bool IsChannelWrite(const IResourceClass &rc) {
  return (rc.GetName() == kChannelWrite);
}

bool IsMapped(const IResourceClass &rc) {
  return (rc.GetName() == kMapped);
}

bool IsEmbedded(const IResourceClass &rc) {
  return (rc.GetName() == kEmbedded);
}

bool IsSiblingTask(const IResourceClass &rc) {
  return (rc.GetName() == kSiblingTask);
}

bool IsSiblingTaskCall(const IResourceClass &rc) {
  return (rc.GetName() == kSiblingTaskCall);
}

bool IsSubModuleTaskCall(const IResourceClass &rc) {
  return (rc.GetName() == kSubModuleTaskCall);
}

bool IsSubModuleTask(const IResourceClass &rc) {
  return (rc.GetName() == kSubModuleTask);
}

bool IsForeignRegister(const IResourceClass &rc) {
  return (rc.GetName() == kForeignReg);
}

static void InstallResource(IDesign *design, const string &name,
			    bool is_exclusive) {
  IResourceClass *klass = new IResourceClass(name, is_exclusive, design);
  design->resource_classes_.push_back(klass);
  ObjectPool *pool = design->GetObjectPool();
  pool->resource_classes_.Add(klass);
}

void InstallResourceClasses(IDesign *design) {
  InstallResource(design, resource::kSet, false);
  InstallResource(design, resource::kPhi, false);
  InstallResource(design, resource::kPseudo, false);
  InstallResource(design, resource::kSelect, false);
  InstallResource(design, resource::kPrint, false);
  InstallResource(design, resource::kAssert, false);
  InstallResource(design, resource::kMapped, true);
  InstallResource(design, resource::kChannelWrite, true);
  InstallResource(design, resource::kChannelRead, false);
  InstallResource(design, resource::kSiblingTask, true);
  InstallResource(design, resource::kSiblingTaskCall, true);
  InstallResource(design, resource::kSubModuleTaskCall, true);
  InstallResource(design, resource::kSubModuleTask, true);
  InstallResource(design, resource::kTransition, true);
  InstallResource(design, resource::kEmbedded, true);
  InstallResource(design, resource::kForeignReg, true);
  InstallResource(design, resource::kExtInput, true);
  InstallResource(design, resource::kExtOutput, true);
  InstallResource(design, resource::kSharedReg, true);
  InstallResource(design, resource::kSharedRegReader, true);
  InstallResource(design, resource::kArray, true);
  InstallResource(design, resource::kGt, true);
  InstallResource(design, resource::kGte, true);
  InstallResource(design, resource::kEq, true);
  InstallResource(design, resource::kAdd, true);
  InstallResource(design, resource::kSub, true);
  InstallResource(design, resource::kMul, true);
  InstallResource(design, resource::kBitAnd, false);
  InstallResource(design, resource::kBitOr, false);
  InstallResource(design, resource::kBitXor, false);
  InstallResource(design, resource::kBitInv, false);
  InstallResource(design, resource::kShift, false);
  InstallResource(design, resource::kBitSel, false);
  InstallResource(design, resource::kBitConcat, false);
  InstallResource(design, resource::kDataFlowIn, false);
}

}  // namespace resource
}  // namespace iroha
