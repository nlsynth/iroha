// -*- C++ -*-
#ifndef _iroha_i_design_h_
#define _iroha_i_design_h_

#include "iroha/common.h"

namespace iroha {

// Type of resource.
// e.g. 'adder' is a resource class and '32 bit adder' is a resource.
class IResourceClass {
public:
  IResourceClass(const string &name, bool is_exclusive, IDesign *design);
  IDesign *GetDesign();
  bool IsExclusive();
  const string &GetName() const;

private:
  const string name_;
  IDesign *design_;
  bool is_exclusive_;
};

class IResource {
public:
  IResource(ITable *table, IResourceClass *resource_class);
  ITable *GetTable();
  IResourceClass *GetClass() const;

private:
  ITable *table_;
  IResourceClass *resource_class_;
};

// Value type. This object is not managed by memory pool.
class IValueType {
public:
  IValueType();

  // Width 0 means a scalar value e.g. 'reg v;'.
  int GetWidth();
  void SetWidth(int width);
  bool IsEnum();
  void SetIsEnum(bool is_enum);

private:
  int width_;
  bool is_enum_;
};

// Literal value. This object is not managed by memory pool.
class IValue {
public:
  IValue();
  uint64_t value_;
  IValueType type_;
};

class IRegister {
public:
  IRegister(ITable *table, const string &name);
  ITable *GetTable() const;
  const string &GetName() const;

  void SetInitialValue(IValue &value);
  const IValue &GetInitialValue() const;
  bool HasInitialValue() const;

  IValueType value_type_;
  bool state_local_;
  bool is_const_;

private:
  ITable *table_;
  string name_;
  IValue initial_value_;
  bool has_initial_value_;
};

class IInsn {
public:
  IInsn(IResource *resource);
  IResource *GetResource() const;

  vector<IRegister *> inputs_;
  vector<IRegister *> outputs_;
  vector<IState *> target_states_;

private:
  IResource *resource_;
};

// This can be a state in an FSM or a pipeline stage.
class IState {
public:
  IState(ITable *table);
  ITable *GetTable();
  int GetId() const;
  void SetId(int id);

  vector<IInsn *> insns_;

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
  vector<IRegister *> registers_;

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
  vector<IResourceClass *> resource_classes_;

private:
  ObjectPool *objects_;
};

}  // namespace iroha

#endif  // _iroha_i_design_h_
