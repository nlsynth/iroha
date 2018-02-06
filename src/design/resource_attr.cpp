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
      resource::IsChannelRead(rc) ||
      resource::IsChannelWrite(rc) ||
      resource::IsSharedMemory(rc) ||
      resource::IsSharedMemoryReader(rc) ||
      resource::IsSharedMemoryWriter(rc) ||
      resource::IsEmbedded(rc) ||
      resource::IsExtTaskDone(rc) ||
      resource::IsExtTaskCall(rc) ||
      resource::IsExtTaskWait(rc)) {
    return true;
  }
  if (resource::IsAxiMasterPort(rc) ||
      resource::IsAxiSlavePort(rc)) {
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
      resource::IsExtOutput(rc) ||
      resource::IsAxiMasterPort(rc) ||
      resource::IsAxiSlavePort(rc) ||
      resource::IsExtTask(rc) ||
      resource::IsExtTaskDone(rc) ||
      resource::IsExtTaskCall(rc) ||
      resource::IsExtTaskWait(rc)) {
    return true;
  }
  if (resource::IsChannelRead(rc) ||
      resource::IsChannelWrite(rc)) {
    IChannel *ch = res->GetChannel();
    if (ch->GetReader() == nullptr ||
	ch->GetWriter() == nullptr) {
      return true;
    }
  }
  return false;
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

}  // namespace iroha
