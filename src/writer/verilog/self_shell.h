// -*- C++ -*-
#ifndef _writer_verilog_self_shell_h_
#define _writer_verilog_self_shell_h_

#include "writer/verilog/common.h"

namespace iroha {
namespace writer {
namespace verilog {

class SelfShell {
 public:
  SelfShell(const IDesign *design, const PortSet *ports, bool reset_polarity,
            EmbeddedModules *embedded_modules);

  void WriteWireDecl(ostream &os);
  void WritePortConnection(ostream &os);
  void WriteShellFSM(ostream &os);

 private:
  void ProcessModule(IModule *mod);
  void ProcessResource(IResource *res);
  void GenConnection(const string &sig, ostream &os);

  const IDesign *design_;
  const PortSet *ports_;
  bool reset_polarity_;
  EmbeddedModules *embedded_modules_;
  vector<IResource *> axi_;
  vector<IResource *> ext_input_;
  vector<IResource *> ext_output_;
  vector<IResource *> ext_task_entry_;
  vector<IResource *> ext_task_call_;
  vector<IResource *> ext_task_wait_;
  vector<IResource *> ext_ram_;
  vector<IResource *> sram_if_;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_self_shell_h_
