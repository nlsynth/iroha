// -*- C++ -*-
//
// Build Iroha design *from* S-Expression.
//
#ifndef _builder_exp_builder_h_
#define _builder_exp_builder_h_

#include "iroha/common.h"
#include "iroha/iroha.h"

namespace iroha {

class Exp;

class ExpBuilder {
public:
  ExpBuilder();

  IDesign *Build(vector<Exp *> &exps);

  static IDesign *ReadDesign(const string &fn);

private:
  IModule *BuildModule(Exp *e, IDesign *design);
  ITable *BuildTable(Exp *e, IModule *module);
  IState *BuildState(Exp *e, ITable *table);
  void SetError();
  bool HasError();

  bool has_error_;
};

}  // namespace iroha

#endif  // _builder_exp_builder_h_
