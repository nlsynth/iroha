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

  static Resource *Create(const IResource &res, const Table &tab);

  virtual void BuildResource();

private:
  void BuildEmbedded();
  void BuildExclusiveBinOp();
  void BuildMapped();
  void BuildSRAM();
  void BuildArray();
  void BuildForeignRegister();
  void CollectResourceCallers(const string &opr,
			      map<IState *, IInsn *> *callers);
  void WriteInputSel(const string &name,
		     const map<IState *, IInsn *> &callers,
		     int nth,
		     ostream &os);
  void WriteWire(const string &name, const IValueType &type,
		 ostream &os);
  void WriteStateUnion(const map<IState *, IInsn *> &callers,
		       ostream &os);

protected:
  string JoinStates(const vector<IState *> &sts);

  const IResource &res_;
  const Table &tab_;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_resource_h_
