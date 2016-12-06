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
const char kMapped[] = "mapped";
const char kChannelWrite[] = "channel-write";
const char kChannelRead[] = "channel-read";
const char kSiblingTask[] = "sibling-task";
const char kSiblingTaskCall[] = "sibling-task-call";
const char kTask[] = "task";
const char kTaskCall[] = "task-call";
const char kExtInput[] = "ext-input";
const char kExtOutput[] = "ext-output";
const char kSharedReg[] = "shared-reg";
const char kSharedRegReader[] = "shared-reg-reader";
const char kSharedRegWriter[] = "shared-reg-writer";
const char kArray[] = "array";
const char kEmbedded[] = "embedded";
const char kForeignReg[] = "foreign-reg";
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
bool IsSet(const IResourceClass &rc);
bool IsBitSel(const IResourceClass &rc);
bool IsBitConcat(const IResourceClass &rc);
bool IsExtInput(const IResourceClass &rc);
bool IsExtOutput(const IResourceClass &rc);
bool IsSharedReg(const IResourceClass &rc);
bool IsSharedRegReader(const IResourceClass &rc);
bool IsSharedRegWriter(const IResourceClass &rc);
bool IsMapped(const IResourceClass &rc);
bool IsChannelRead(const IResourceClass &rc);
bool IsChannelWrite(const IResourceClass &rc);
bool IsEmbedded(const IResourceClass &rc);
bool IsTask(const IResourceClass &rc);
bool IsTaskCall(const IResourceClass &rc);
bool IsSiblingTask(const IResourceClass &rc);
bool IsSiblingTaskCall(const IResourceClass &rc);
bool IsForeignRegister(const IResourceClass &rc);

void InstallResourceClasses(IDesign *design);

}  // namespace resource
}  // namespace iroha

#endif  // _iroha_resource_class_h_
