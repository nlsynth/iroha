#include "iroha/resource_class.h"

#include "iroha/i_design.h"
#include "iroha/object_pool.h"
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

bool IsExtCombinational(const IResourceClass &rc) {
  return (rc.GetName() == kExtCombinational);
}

bool IsSharedReg(const IResourceClass &rc) {
  return (rc.GetName() == kSharedReg);
}

bool IsSharedRegReader(const IResourceClass &rc) {
  return (rc.GetName() == kSharedRegReader);
}

bool IsSharedRegWriter(const IResourceClass &rc) {
  return (rc.GetName() == kSharedRegWriter);
}

bool IsSharedRegExtWriter(const IResourceClass &rc) {
  return (rc.GetName() == kSharedRegExtWriter);
}

bool IsSharedMemory(const IResourceClass &rc) {
  return (rc.GetName() == kSharedMemory);
}

bool IsSharedMemoryReader(const IResourceClass &rc) {
  return (rc.GetName() == kSharedMemoryReader);
}

bool IsSharedMemoryReplica(const IResourceClass &rc) {
  return (rc.GetName() == kSharedMemoryReplica);
}

bool IsSharedMemoryWriter(const IResourceClass &rc) {
  return (rc.GetName() == kSharedMemoryWriter);
}

bool IsTask(const IResourceClass &rc) {
  return (rc.GetName() == kTask);
}

bool IsTaskCall(const IResourceClass &rc) {
  return (rc.GetName() == kTaskCall);
}

bool IsDataFlowIn(const IResourceClass &rc) {
  return (rc.GetName() == kDataFlowIn);
}

bool IsExtFlowCall(const IResourceClass &rc) {
  return (rc.GetName() == kExtFlowCall);
}

bool IsExtFlowResult(const IResourceClass &rc) {
  return (rc.GetName() == kExtFlowResult);
}

bool IsExtTask(const IResourceClass &rc) {
  return (rc.GetName() == kExtTask);
}

bool IsExtTaskDone(const IResourceClass &rc) {
  return (rc.GetName() == kExtTaskDone);
}

bool IsExtTaskCall(const IResourceClass &rc) {
  return (rc.GetName() == kExtTaskCall);
}

bool IsExtTaskWait(const IResourceClass &rc) {
  return (rc.GetName() == kExtTaskWait);
}

bool IsAxiMasterPort(const IResourceClass &rc) {
  return (rc.GetName() == kAxiMasterPort);
}

bool IsAxiSlavePort(const IResourceClass &rc) {
  return (rc.GetName() == kAxiSlavePort);
}

bool IsFifo(const IResourceClass &rc) {
  return (rc.GetName() == kFifo);
}

bool IsFifoReader(const IResourceClass &rc) {
  return (rc.GetName() == kFifoReader);
}

bool IsFifoWriter(const IResourceClass &rc) {
  return (rc.GetName() == kFifoWriter);
}

bool IsSramIf(const IResourceClass &rc) {
  return (rc.GetName() == kSramIf);
}

bool IsTicker(const IResourceClass &rc) {
  return (rc.GetName() == kTicker);
}

bool IsStudy(const IResourceClass &rc) {
  return (rc.GetName() == kStudy);
}

bool IsStudyReader(const IResourceClass &rc) {
  return (rc.GetName() == kStudyReader);
}

bool IsStudyWriter(const IResourceClass &rc) {
  return (rc.GetName() == kStudyWriter);
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
  InstallResource(design, resource::kTask, true);
  InstallResource(design, resource::kTaskCall, true);
  InstallResource(design, resource::kTransition, true);
  InstallResource(design, resource::kExtInput, true);
  InstallResource(design, resource::kExtOutput, true);
  InstallResource(design, resource::kExtCombinational, true);
  InstallResource(design, resource::kSharedReg, true);
  InstallResource(design, resource::kSharedRegReader, true);
  InstallResource(design, resource::kSharedRegWriter, true);
  InstallResource(design, resource::kSharedRegExtWriter, true);
  InstallResource(design, resource::kSharedMemory, true);
  InstallResource(design, resource::kSharedMemoryReader, true);
  InstallResource(design, resource::kSharedMemoryReplica, true);
  InstallResource(design, resource::kSharedMemoryWriter, true);
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
  InstallResource(design, resource::kExtFlowCall, true);
  InstallResource(design, resource::kExtFlowResult, true);
  InstallResource(design, resource::kExtTask, true);
  InstallResource(design, resource::kExtTaskDone, true);
  InstallResource(design, resource::kExtTaskCall, true);
  InstallResource(design, resource::kExtTaskWait, true);
  InstallResource(design, resource::kAxiMasterPort, true);
  InstallResource(design, resource::kAxiSlavePort, true);
  InstallResource(design, resource::kFifo, true);
  InstallResource(design, resource::kFifoReader, true);
  InstallResource(design, resource::kFifoWriter, true);
  InstallResource(design, resource::kTicker, true);
  InstallResource(design, resource::kSramIf, true);
  InstallResource(design, resource::kStudy, true);
  InstallResource(design, resource::kStudyReader, true);
  InstallResource(design, resource::kStudyWriter, true);
}

}  // namespace resource
}  // namespace iroha
