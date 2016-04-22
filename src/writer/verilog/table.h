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
  string StateName(int id);
  ModuleTemplate *GetModuleTemplate() const;
  string InitialStateName();
  bool IsTask();
  bool IsEmpty();
  Ports *GetPorts() const;
  Embed *GetEmbed() const;
  Module *GetModule() const;
  Task *GetTask() const;
  string SharedRegPrefix(const ITable &writer, const IRegister &reg) const;
  static string WidthSpec(const IRegister *reg);

private:
  void BuildStateDecl();
  void BuildResource();
  void BuildRegister();
  void BuildInsnOutputWire();
  void BuildSharedRegisters();

  ITable *i_table_;
  Ports *ports_;
  Module *mod_;
  Embed *embed_;
  ModuleTemplate *tmpl_;
  int table_id_;
  string st_;
  vector<State *> states_;
  unique_ptr<Task> task_;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_table_h_
