// -*- C++ -*-
#ifndef _writer_verilog_table_h_
#define _writer_verilog_table_h_

#include "writer/verilog/common.h"

#include <map>

namespace iroha {
namespace writer {
namespace verilog {

class Table {
public:
  Table(ITable *table, Ports *ports, Module *mod, EmbeddedModules *embed,
	Names *names, ModuleTemplate *tmpl);
  virtual ~Table();

  void CollectNames();
  virtual void Build();
  virtual void Write(ostream &os);

  // Returns a string to test if current state == st.
  virtual string GetStateCondition(const IState *st) const;
  virtual const DataFlowTable *GetDataFlowTable() const;

  ITable *GetITable() const;
  const string &StateVariable() const;
  string StateName(int id) const;
  string InitialStateName();
  bool IsTaskOrExtTask();
  bool IsEmpty();

  Ports *GetPorts() const;
  EmbeddedModules *GetEmbeddedModules() const;
  Module *GetModule() const;
  Task *GetTask() const;
  ModuleTemplate *GetModuleTemplate() const;
  Names *GetNames() const;

  ostream &StateOutputSectionStream() const;
  string StateOutputSectionContents() const;
  ostream &InitialValueSectionStream() const;
  string InitialValueSectionContents() const;
  ostream &TaskEntrySectionStream() const;
  string TaskEntrySectionContents() const;
  ostream &ResourceSectionStream() const;
  string ResourceSectionContents() const;
  ostream &RegisterSectionStream() const;
  string RegisterSectionContents() const;
  ostream &InsnWireDeclSectionStream() const;
  string InsnWireDeclSectionContents() const;
  ostream &InsnWireValueSectionStream() const;
  string InsnWireValueSectionContents() const;

  static string ValueWidthSpec(const IValueType &type);
  static string WidthSpec(int w);
  static string StateNameFromTable(const ITable &tab, int id);

private:
  virtual void BuildStates();
  virtual void BuildStateDecl();
  void BuildResource();
  void BuildRegister();
  void BuildInsnOutputWire();
  void BuildMultiCycleStateReg();

  virtual void WriteReset(ostream &os);
  virtual void WriteBody(ostream &os);

protected:
  ITable *i_table_;
  Ports *ports_;
  Module *mod_;
  EmbeddedModules *embedded_modules_;
  Names *names_;
  ModuleTemplate *tmpl_;
  int table_id_;
  string st_;
  vector<State *> states_;
  bool is_task_or_ext_task_;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_table_h_
