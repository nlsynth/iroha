// -*- C++ -*-
#ifndef _writer_verilog_verilog_writer_h_
#define _writer_verilog_verilog_writer_h_

#include "writer/verilog/common.h"

#include <map>

namespace iroha {
namespace writer {
namespace verilog {

class VerilogWriter {
public:
  VerilogWriter(const IDesign *design, const Connection &conn, ostream &os);
  ~VerilogWriter();

  void Write();
  void SetShellModuleName(const string &n);

private:
  void BuildModules(const IModule *imod);
  void BuildHierarchy();
  void WriteShellModule(const Module *mod);

  const IDesign *design_;
  const Connection &conn_;
  ostream &os_;
  vector<Module *> ordered_modules_;
  map<const IModule *, Module *> modules_;
  unique_ptr<EmbeddedModules> embedded_modules_;
  string shell_module_name_;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_verilog_writer_h_
