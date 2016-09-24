// -*- C++ -*-
#ifndef _design_validator_h_
#define _design_validator_h_

#include "iroha/common.h"

namespace iroha {

class Validator {
public:
  static void Validate(IDesign *design);
  static void ValidateTable(ITable *table);

private:
  static void ValidateChannelId(IDesign *design);
  static void ValidateArrayImageId(IDesign *design);
  static void ValidateModuleId(IDesign *design);
  static void ValidateStateId(ITable *table);
  static void ValidateTableId(IModule *mod);
  static void ValidateInsnId(ITable *table);
  static void ValidateRegisterId(ITable *table);
  static void ValidateResourceId(ITable *table);
};

}  // namespace iroha

#endif  // _design_validator_h_
