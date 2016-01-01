// -*- C++ -*-
#ifndef _iroha_resource_class_h_
#define _iroha_resource_class_h_

#include "iroha/common.h"

namespace iroha {
namespace resource {
// NOTE(yt76): Enums instead of string?
const char kTransition[] = "tr";
const char kSet[] = "set";
const char kSubModuleTaskCall[] = "sub_module_task_call";
const char kSubModuleTask[] = "sub_module_task";
const char kExtInput[] = "ext_input";
const char kExtOutput[] = "ext_output";
const char kArray[] = "array";
const char kEmbedded[] = "embedded";
const char kAdd[] = "add";
const char kSub[] = "sub";
const char kGt[] = "gt";

bool IsBinOp(const IResourceClass &rc);
bool IsNumToNumBinOp(const IResourceClass &rc);
bool IsNumToBoolBinOp(const IResourceClass &rc);
bool IsArray(const IResourceClass &rc);
bool IsSubModuleTaskCall(const IResourceClass &rc);
bool IsSubModuleTask(const IResourceClass &rc);

}  // namespace resource
}  // namespace iroha

#endif  // _iroha_resource_class_h_
