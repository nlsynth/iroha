// -*- C++ -*-
#ifndef _iroha_resource_class_h_
#define _iroha_resource_class_h_

#include "iroha/common.h"

namespace iroha {
namespace resource {
const char kTransition[] = "tr";
const char kSet[] = "set";
const char kPrint[] = "print";
const char kPhi[] = "phi";
const char kPseudo[] = "pseudo";
const char kSelect[] = "select";
const char kAssert[] = "assert";
const char kTask[] = "task";
const char kTaskCall[] = "task-call";
const char kExtInput[] = "ext-input";
const char kExtOutput[] = "ext-output";
const char kExtInputAccessor[] = "ext-input-accessor";
const char kExtOutputAccessor[] = "ext-output-accessor";
const char kExtCombinational[] = "ext-combinational";
const char kSharedReg[] = "shared-reg";
const char kSharedRegReader[] = "shared-reg-reader";
const char kSharedRegWriter[] = "shared-reg-writer";
const char kSharedRegExtWriter[] = "shared-reg-ext-writer";
const char kSharedMemory[] = "shared-memory";
const char kSharedMemoryReader[] = "shared-memory-reader";
const char kSharedMemoryReplica[] = "shared-memory-replica";
const char kSharedMemoryWriter[] = "shared-memory-writer";
const char kArray[] = "array";
const char kArrayRData[] = "array-rdata";
const char kAdd[] = "add";
const char kSub[] = "sub";
const char kMul[] = "mul";
const char kBitAnd[] = "bit-and";
const char kBitOr[] = "bit-or";
const char kBitXor[] = "bit-xor";
const char kBitInv[] = "bit-inv";
const char kGt[] = "gt";
const char kGte[] = "gte";
const char kEq[] = "eq";
const char kShift[] = "shift";
const char kBitSel[] = "bit-sel";
const char kBitConcat[] = "bit-concat";
const char kDataFlowIn[] = "dataflow-in";
const char kExtFlowCall[] = "ext-flow-call";
const char kExtFlowResult[] = "ext-flow-result";
const char kExtTask[] = "ext-task";
const char kExtTaskDone[] = "ext-task-done";
const char kExtTaskCall[] = "ext-task-call";
const char kExtTaskWait[] = "ext-task-wait";
const char kAxiMasterPort[] = "axi-master-port";
const char kAxiSlavePort[] = "axi-slave-port";
const char kFifo[] = "fifo";
const char kFifoReader[] = "fifo-reader";
const char kFifoWriter[] = "fifo-writer";
const char kTicker[] = "ticker";
const char kTickerAccessor[] = "ticker-accessor";
const char kSramIf[] = "sram-if";
const char kStudy[] = "study";
const char kStudyReader[] = "study-reader";
const char kStudyWriter[] = "study-writer";

bool IsPrint(const IResourceClass &rc);
bool IsAssert(const IResourceClass &rc);
bool IsTransition(const IResourceClass &rc);
bool IsSelect(const IResourceClass &rc);
bool IsExclusiveBinOp(const IResourceClass &rc);
bool IsPseudo(const IResourceClass &rc);
bool IsNumToNumExclusiveBinOp(const IResourceClass &rc);
bool IsNumToBoolExclusiveBinOp(const IResourceClass &rc);
bool IsLightBinOp(const IResourceClass &rc);
bool IsLightUniOp(const IResourceClass &rc);
bool IsBitShiftOp(const IResourceClass &rc);
bool IsArray(const IResourceClass &rc);
bool IsArrayRData(const IResourceClass &rc);
bool IsSet(const IResourceClass &rc);
bool IsAdd(const IResourceClass &rc);
bool IsGt(const IResourceClass &rc);
bool IsBitSel(const IResourceClass &rc);
bool IsBitConcat(const IResourceClass &rc);
bool IsExtInput(const IResourceClass &rc);
bool IsExtOutput(const IResourceClass &rc);
bool IsExtInputAccessor(const IResourceClass &rc);
bool IsExtOutputAccessor(const IResourceClass &rc);
bool IsExtCombinational(const IResourceClass &rc);
bool IsSharedReg(const IResourceClass &rc);
bool IsSharedRegReader(const IResourceClass &rc);
bool IsSharedRegWriter(const IResourceClass &rc);
bool IsSharedRegExtWriter(const IResourceClass &rc);
bool IsSharedMemory(const IResourceClass &rc);
bool IsSharedMemoryReader(const IResourceClass &rc);
bool IsSharedMemoryReplica(const IResourceClass &rc);
bool IsSharedMemoryWriter(const IResourceClass &rc);
bool IsTask(const IResourceClass &rc);
bool IsTaskCall(const IResourceClass &rc);
bool IsForeignRegister(const IResourceClass &rc);
bool IsDataFlowIn(const IResourceClass &rc);
bool IsExtFlowCall(const IResourceClass &rc);
bool IsExtFlowResult(const IResourceClass &rc);
bool IsExtTask(const IResourceClass &rc);
bool IsExtTaskDone(const IResourceClass &rc);
bool IsExtTaskCall(const IResourceClass &rc);
bool IsExtTaskWait(const IResourceClass &rc);
bool IsAxiMasterPort(const IResourceClass &rc);
bool IsAxiSlavePort(const IResourceClass &rc);
bool IsFifo(const IResourceClass &rc);
bool IsFifoReader(const IResourceClass &rc);
bool IsFifoWriter(const IResourceClass &rc);
bool IsSramIf(const IResourceClass &rc);
bool IsTicker(const IResourceClass &rc);
bool IsTickerAccessor(const IResourceClass &rc);
bool IsStudy(const IResourceClass &rc);
bool IsStudyReader(const IResourceClass &rc);
bool IsStudyWriter(const IResourceClass &rc);

void InstallResourceClasses(IDesign *design);

}  // namespace resource
}  // namespace iroha

#endif  // _iroha_resource_class_h_
