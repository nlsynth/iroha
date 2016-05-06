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
	ModuleTemplate *tmpl);
  ~Table();
  void Build();

  void Write(ostream &os);
  ITable *GetITable() const;
  const string &StateVariable() const;
  string StateName(int id) const;
  string InitialStateName();
  bool IsTask();
  bool IsEmpty();

  Ports *GetPorts() const;
  EmbeddedModules *GetEmbeddedModules() const;
  Module *GetModule() const;
  Task *GetTask() const;
  ModuleTemplate *GetModuleTemplate() const;

  ostream &StateOutputSectionStream() const;
  string StateOutputSectionContents() const;
  ostream &InitialValueSectionStream() const;
  string InitialValueSectionContents() const;
  ostream &TaskEntrySectionStream() const;
  string TaskEntrySectionContents() const;

  static string WidthSpec(const IValueType &type);
  static string StateNameFromTable(const ITable &tab, int id);

private:
  void BuildStateDecl();
  void BuildResource();
  void BuildRegister();
  void BuildInsnOutputWire();

  ITable *i_table_;
  Ports *ports_;
  Module *mod_;
  EmbeddedModules *embedded_modules_;
  ModuleTemplate *tmpl_;
  int table_id_;
  string st_;
  vector<State *> states_;
  bool is_task_;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_table_h_
