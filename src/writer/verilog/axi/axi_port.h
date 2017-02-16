// -*- C++ -*-
#ifndef _writer_verilog_axi_axi_port_h_
#define _writer_verilog_axi_axi_port_h_

#include "writer/verilog/resource.h"

namespace iroha {
namespace writer {
namespace verilog {
namespace axi {

class AxiPort : public Resource {
public:
  AxiPort(const IResource &res, const Table &table);

  virtual void BuildResource();
  virtual void BuildInsn(IInsn *insn, State *st);

  static void GetReadWrite(const IResource &res, bool *r, bool *w);

  static string ControllerName(const IResource &res, bool reset_polarity);
  static void WriteController(const IResource &res,
			      bool reset_polarity, ostream &os);

private:
  void BuildInstance(const string &s);
  string BuildPort();
};

}  // namespace axi
}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_axi_axi_port_h_


