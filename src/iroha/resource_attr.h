// -*- C++ -*-
#ifndef _iroha_resource_attr_h_
#define _iroha_resource_attr_h_

#include "iroha/common.h"

namespace iroha {

class ResourceAttr {
 public:
  static bool IsMultiCycleInsn(IInsn *insn);
  static int NumMultiCycleInsn(const IState *st);
  static bool IsExtAccessResource(IResource *res);
  static bool IsExtAccessInsn(IInsn *insn);
  static bool IsExtWaitInsn(IInsn *insn);
  static bool IsSideEffectInsn(IInsn *insn);
  static int NumExtAccessInsn(const IState *st);
  static bool IsDuplicatableResource(IResource *res);
  static bool IsOrderedResource(IResource *res);
};

}  // namespace iroha

#endif  // _iroha_resource_attr_h_
