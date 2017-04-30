// -*- C++ -*-
#ifndef _writer_verilog_axi_slave_port_h_
#define _writer_verilog_axi_slave_port_h_

#include "writer/verilog/resource.h"

namespace iroha {
namespace writer {
namespace verilog {
namespace axi {

class SlavePort : public Resource {
public:
  SlavePort(const IResource &res, const Table &table);

  virtual void BuildResource();
  virtual void BuildInsn(IInsn *insn, State *st);

  static string ControllerName(const IResource &res, bool reset_polarity);
  static void WriteController(const IResource &res,
			      bool reset_polarity, ostream &os);
};

}  // namespace axi
}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_axi_slave_port_h_
