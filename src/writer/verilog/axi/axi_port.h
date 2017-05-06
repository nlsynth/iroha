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

protected:
  void OutputSRAMConnection(ostream &os);

  string PortSuffix();
  string AddrPort();
  string WenPort();
  string ReqPort();
  string AckPort();

  bool reset_polarity_;
};

}  // namespace axi
}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_axi_axi_port_h_
