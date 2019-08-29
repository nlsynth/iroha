// -*- C++ -*-
#ifndef _writer_verilog_self_shell_h_
#define _writer_verilog_self_shell_h_

#include "writer/verilog/common.h"

namespace iroha {
namespace writer {
namespace verilog {

class SelfShell {
public:
  SelfShell(const IDesign *design, const Ports *ports, bool reset_polarity);

  void WriteWireDecl(ostream &os);
  void WritePortConnection(ostream &os);
  void WriteShellFSM(ostream &os);

private:
  void ProcessModule(IModule *mod);

  const IDesign *design_;
  const Ports *ports_;
  bool reset_polarity_;
  vector<IResource *> axi_;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_self_shell_h_
