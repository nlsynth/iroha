// -*- C++ -*-
#ifndef _writer_verilog_axi_master_port_h_
#define _writer_verilog_axi_master_port_h_

#include "writer/verilog/axi/axi_port.h"

namespace iroha {
namespace writer {
namespace verilog {
namespace axi {

class MasterPort : public AxiPort {
public:
  MasterPort(const IResource &res, const Table &table);

  virtual void BuildResource();
  virtual void BuildInsn(IInsn *insn, State *st);

  static void GetReadWrite(const IResource &res, bool *r, bool *w);

  static string ControllerName(const IResource &res, bool reset_polarity);
  static void WriteController(const IResource &res,
			      bool reset_polarity, ostream &os);

private:
  void BuildInstance(const string &s);
  string BuildPortToExt();

  string PortSuffix();
  string AddrPort();
  string WenPort();
  string ReqPort();
  string AckPort();
};

}  // namespace axi
}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_axi_master_port_h_
