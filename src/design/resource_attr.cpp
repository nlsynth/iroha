#include "design/resource_attr.h"

#include "iroha/i_design.h"
#include "iroha/insn_operands.h"
#include "iroha/resource_class.h"

namespace iroha {

int ResourceAttr::NumMultiCycleInsn(const IState *st) {
  int n = 0;
  for (IInsn *insn : st->insns_) {
    if (IsMultiCycleInsn(insn)) {
      ++n;
    }
  }
  return n;
}

bool ResourceAttr::IsMultiCycleInsn(IInsn *insn) {
  IResource *res = insn->GetResource();
  IResourceClass &rc = *(res->GetClass());
  if (resource::IsTaskCall(rc) ||
      resource::IsSharedMemory(rc) ||
      resource::IsSharedMemoryReader(rc) ||
      resource::IsSharedMemoryWriter(rc) ||
      resource::IsExtTaskDone(rc) ||
      resource::IsExtTaskCall(rc) ||
      resource::IsExtTaskWait(rc)) {
    return true;
  }
  if (resource::IsStudy(rc) ||
      resource::IsStudyReader(rc) ||
      resource::IsStudyWriter(rc)) {
    return true;
  }
  if (resource::IsAxiMasterPort(rc) ||
      resource::IsAxiSlavePort(rc) ||
      resource::IsSramIf(rc)) {
    return true;
  }
  if (resource::IsFifo(rc) ||
      resource::IsFifoReader(rc)) {
    return true;
  }
  if (resource::IsFifoWriter(rc) &&
      insn->GetOperand() != operand::kNoWait) {
    return true;
  }
  if (resource::IsSharedRegReader(rc)) {
    if (insn->GetOperand() == operand::kWaitNotify ||
	insn->GetOperand() == operand::kGetMailbox) {
      return true;
    }
  }
  if (resource::IsSharedRegWriter(rc)) {
    if (insn->GetOperand() == operand::kPutMailbox) {
      return true;
    }
  }
  return false;
}

bool ResourceAttr::IsExtAccessResource(IResource *res) {
  IResourceClass &rc = *(res->GetClass());
  if (resource::IsExtInput(rc) ||
      resource::IsExtInputAccessor(rc) ||
      resource::IsExtOutput(rc) ||
      resource::IsExtOutputAccessor(rc) ||
      resource::IsAxiMasterPort(rc) ||
      resource::IsAxiSlavePort(rc) ||
      resource::IsSramIf(rc) ||
      resource::IsExtTask(rc) ||
      resource::IsExtTaskDone(rc) ||
      resource::IsExtTaskCall(rc) ||
      resource::IsExtTaskWait(rc) ||
      resource::IsSharedRegExtWriter(rc)) {
    return true;
  }
  if (resource::IsArray(rc) && res->GetArray() != nullptr &&
      res->GetArray()->IsExternal()) {
    return true;
  }
  return false;
}

bool ResourceAttr::IsExtWaitInsn(IInsn *insn) {
  IResourceClass &rc = *(insn->GetResource()->GetClass());
  return resource::IsAxiSlavePort(rc) || resource::IsSramIf(rc);
}

bool ResourceAttr::IsExtAccessInsn(IInsn *insn) {
  return IsExtAccessResource(insn->GetResource());
}

int ResourceAttr::NumExtAccessInsn(const IState *st) {
  int n = 0;
  for (IInsn *insn : st->insns_) {
    if (IsExtAccessInsn(insn)) {
      ++n;
    }
  }
  return n;
}

bool ResourceAttr::IsDuplicatableResource(IResource *res) {
  IResourceClass &rc = *(res->GetClass());
  const string &name = rc.GetName();
  // add, sub, mul, gt, gte and ext-combinational for now.
  if (name == resource::kEq) {
    return false;
  }
  if (resource::IsExtCombinational(rc)) {
    return true;
  }
  return resource::IsExclusiveBinOp(rc);
}

bool ResourceAttr::IsOrderedResource(IResource *res) {
  IResourceClass &rc = *(res->GetClass());
  if (resource::IsArray(rc) || resource::IsSharedMemoryReplica(rc)) {
    return true;
  }
  return false;
}

}  // namespace iroha
