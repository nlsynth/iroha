// -*- C++ -*-
#ifndef _writer_verilog_axi_axi_shell_h_
#define _writer_verilog_axi_axi_shell_h_

#include "writer/verilog/common.h"

namespace iroha {
namespace writer {
namespace verilog {
namespace axi {

class AxiShell {
public:
  AxiShell(IResource *res);

  void WriteWireDecl(ostream &os);
  void WritePortConnection(ostream &os);

private:
  IResource *res_;
};

}  // namespace axi
}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_axi_axi_shell_h_
