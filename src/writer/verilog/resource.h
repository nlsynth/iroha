// -*- C++ -*-
#ifndef _writer_verilog_resource_h_
#define _writer_verilog_resource_h_

#include "writer/verilog/common.h"

#include <map>

namespace iroha {
namespace writer {
namespace verilog {

class Resource {
public:
  Resource(const IResource &res, const Table &tab);
  virtual ~Resource();

  static Resource *Create(const IResource &res, const Table &tab);

  virtual void BuildResource();
  virtual void BuildInsn(IInsn *insn, State *st);
  virtual void CollectNames(Names *names);

  ModuleTemplate *GetModuleTemplate() const;
  const Table &GetTable() const;
  const IResource &GetIResource() const;

  string JoinStatesWithSubState(const map<IState *, IInsn *> &sts, int sub) const;
  void CollectResourceCallers(const string &opr,
			      map<IState *, IInsn *> *callers) const;

protected:
  void WriteInputSel(const string &name,
		     const map<IState *, IInsn *> &callers,
		     int nth,
		     ostream &os);
  void WriteWire(const string &name, const IValueType &type,
		 ostream &os);
  string JoinStates(const map<IState *, IInsn *> &sts) const;
  void WriteStateUnion(const map<IState *, IInsn *> &callers,
		       ostream &os);
  string SelectValueByState(const string &default_value);
  void AddPortToTop(const string &port, bool is_output, bool from_embedded,
		    int width);
  void BuildEmbeddedModule(const string &connection);

  const IResource &res_;
  const Table &tab_;
  ModuleTemplate *tmpl_;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_resource_h_
