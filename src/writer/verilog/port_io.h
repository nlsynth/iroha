// -*- C++ -*-
#ifndef _writer_verilog_port_io_h_
#define _writer_verilog_port_io_h_

#include "writer/verilog/resource.h"

namespace iroha {
namespace writer {
namespace verilog {

class PortIO : public Resource {
public:
  PortIO(const IResource &res, const Table &table);

  virtual void BuildResource() override;
  virtual void BuildInsn(IInsn *insn, State *st) override;

  static string PortName(const IResource &res);

private:
  int width_;
  string output_port_;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_port_io_h_
