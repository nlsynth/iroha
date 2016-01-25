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
	ModuleTemplate *tmpl, int nth);
  ~Table();
  void Build();

  void Write(ostream &os);
  const string &StateVariable() const;
  string StateName(int id);
  ModuleTemplate *GetModuleTemplate() const;

private:
  void BuildStateDecl();
  void BuildResource();
  void BuildRegister();
  void BuildInsnOutputWire();
  void BuildExclusiveBinOpResource(const IResource &res);
  void BuildArrayResource(const IResource &res);
  void BuildMappedResource(const IResource &res);
  void BuildSubModuleTaskResource(const IResource &res);
  void BuildEmbededResource(const IResource &res);
  void BuildSRAMResource(const IResource &res);
  string WidthSpec(const IRegister *reg);

  void CollectResourceCallers(const IResource &res,
			      const string &opr,
			      map<IState *, IInsn *> *callers);
  void WriteWire(const string &name, const IValueType &type,
		 ostream &os);
  void WriteInputSel(const string &name, const IResource &res,
		     const map<IState *, IInsn *> &callers,
		     int nth,
		     ostream &os);
  void WriteStateUnion(const map<IState *, IInsn *> &callers,
		       ostream &os);

  ITable *i_table_;
  Ports *ports_;
  Module *mod_;
  Embed *embed_;
  ModuleTemplate *tmpl_;
  int nth_;
  string st_;
  vector<State *> states_;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_table_h_
