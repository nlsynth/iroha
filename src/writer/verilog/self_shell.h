// -*- C++ -*-
#ifndef _writer_verilog_self_shell_h_
#define _writer_verilog_self_shell_h_

#include "writer/verilog/common.h"

namespace iroha {
namespace writer {
namespace verilog {

class SelfShell {
public:
  SelfShell(const IDesign *design);

  void WriteWireDecl(ostream &os);
  void WritePortConnection(ostream &os);

private:
  void ProcessModule(IModule *mod);

  const IDesign *design_;
  vector<IResource *> axi_;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_self_shell_h_
