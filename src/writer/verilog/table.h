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
  Table(ITable *table, Ports *ports, Module *mod, Embed *embed,
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
  Embed *GetEmbed() const;
  Module *GetModule() const;
  Task *GetTask() const;
  ModuleTemplate *GetModuleTemplate() const;

  ostream &StateOutputSectionStream() const;
  string StateOutputSectionContents() const;
  ostream &InitialValueSectionStream() const;
  string InitialValueSectionContents() const;

  static string WidthSpec(const IRegister *reg);
  static string StateNameFromTable(const ITable &tab, int id);

private:
  void BuildStateDecl();
  void BuildResource();
  void BuildRegister();
  void BuildInsnOutputWire();

  ITable *i_table_;
  Ports *ports_;
  Module *mod_;
  Embed *embed_;
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
