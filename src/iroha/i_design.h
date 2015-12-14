// -*- C++ -*-
#ifndef _iroha_i_design_h_
#define _iroha_i_design_h_

#include "iroha/common.h"

namespace iroha {

class IDesign;
class IModule;
class IState;
class ITable;
class ObjectPool;

class IResource {
public:
};

class IRegister {
public:
  ITable *table_;
  uint64_t initial_value_;
  bool has_initial_value_;
  bool state_local_;
};

class IInsn {
public:
  IState *state_;
  IResource *resource_;
};

// This can be a state in an FSM or a pipeline stage.
class IState {
public:
  IState(ITable *table);
  ITable *GetTable();
  int GetId() const;
  void SetId(int id);

private:
  ITable *table_;
  int id_;
};

// Table is a matrix of resource columns and state rows to define
// execution scheduling.
// It can mean either a pipeline stage or an FSM.
class ITable {
public:
  ITable(IModule *module);
  IModule *GetModule();
  void SetInitialState(IState *state);
  IState *GetInitialState();

  vector<IState *> states_;
  vector<IResource *> resources_;

private:
  IModule *module_;
  IState *initial_state_;
};

// IModule corresponds to a module in HDL.
// This can be either
// (1) An FSM with 1 table.
// (2) A pipeline with multiple tables.
// (3) Container of multiple modules.
class IModule {
public:
  IModule(IDesign *design, const string &name);

  IDesign *GetDesign();
  const string &GetName() const;

  vector<ITable *> tables_;

private:
  IDesign *design_;
  string name_;
};

// Represents a whole design including module hierarchy,
// interfaces, some external stuff, configs and so on.
class IDesign {
public:
  IDesign();
  ~IDesign();

  ObjectPool *GetObjectPool();

  vector<IModule *> modules_;

private:
  ObjectPool *objects_;
};

}  // namespace iroha

#endif  // _iroha_i_design_h_
