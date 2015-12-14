// -*- C++ -*-
#ifndef _iroha_i_design_h_
#define _iroha_i_design_h_

#include "iroha/common.h"

namespace iroha {

class IDesign;
class IModule;
class ITable;
class ObjectPool;

class IResource {
public:
};

class IInsn {
public:
};

class IState {
public:
public:
  IState(ITable *table);
  ITable *table_;
};

class ITable {
public:
  ITable(IModule *module);
  vector<IState *> states_;
  IModule *module_;
};

class IModule {
public:
  IModule(IDesign *design, const string &name);
  IDesign *design_;
  string name_;
  vector<ITable *> tables_;
};

class IDesign {
public:
  IDesign();
  ~IDesign();

  vector<IModule *> modules_;
  ObjectPool *objects_;
};

}  // namespace iroha

#endif  // _iroha_i_design_h_
