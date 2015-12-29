// -*- C++ -*-
//
// Build Iroha design *from* S-Expression.
//
#ifndef _builder_exp_builder_h_
#define _builder_exp_builder_h_

#include "iroha/common.h"

#include <sstream>

namespace iroha {

class Exp;

class ExpBuilder {
public:
  ExpBuilder();

  IDesign *Build(vector<Exp *> &exps);
  ostream &SetError();

  static IDesign *ReadDesign(const string &fn);

private:
  IModule *BuildModule(Exp *e, IDesign *design);
  ITable *BuildTable(Exp *e, IModule *module);
  IState *BuildState(Exp *e, ITable *table);
  void BuildRegisters(Exp *e, ITable *table);
  IRegister *BuildRegister(Exp *e, ITable *table);
  void BuildResources(Exp *e, ITable *table);
  IResource *BuildResource(Exp *e, ITable *table);
  void BuildResourceParams(Exp *e, ResourceParams *params);
  void BuildParamTypes(Exp *e, vector<IValueType> *types);
  bool HasError();

  bool has_error_;
  ostringstream errors_;
};

}  // namespace iroha

#endif  // _builder_exp_builder_h_
