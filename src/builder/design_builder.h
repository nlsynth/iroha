// -*- C++ -*-
//
// Build Iroha design *from* S-Expression.
//
#ifndef _builder_design_builder_h_
#define _builder_design_builder_h_

#include "iroha/common.h"

#include <sstream>

namespace iroha {
namespace builder {

class Exp;
class TreeBuilder;

class DesignBuilder {
public:
  DesignBuilder();
  ~DesignBuilder();

  IDesign *Build(vector<Exp *> &exps);
  ostream &SetError();

  static IDesign *ReadDesign(const string &fn, bool search);

private:
  IModule *BuildModule(Exp *e, IDesign *design);
  ITable *BuildTable(Exp *e, IModule *module);
  IState *BuildState(Exp *e, ITable *table);
  void BuildRegisters(Exp *e, ITable *table);
  IRegister *BuildRegister(Exp *e, ITable *table);
  void BuildResources(Exp *e, ITable *table);
  void BuildArray(Exp *e, IResource *res);
  void BuildArrayImage(Exp *e, IDesign *design);
  IResource *BuildResource(Exp *e, ITable *table);
  void BuildResourceParams(Exp *e, ResourceParams *params);
  void BuildParamTypes(Exp *e, vector<IValueType> *types);
  void BuildValueType(Exp *t, IValueType *vt);
  bool HasError();

  bool has_error_;
  ostringstream errors_;
  unique_ptr<TreeBuilder> tree_builder_;
};

}  // namespace builder
}  // namespace iroha

#endif  // _builder_design_builder_h_
