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

  bool Write();
  void SetShellModuleName(const string &n, bool with_self_clock);
  Module *GetByIModule(IModule *mod) const;

private:
  void PrepareModulesRec(const IModule *imod);
  void BuildModules(const IModule *imod);
  void BuildChildModuleSection();
  void WriteShellModule(const Module *mod);
  void WriteSelfClockGenerator(const Module *mod);
  void WriteSelfClockConnection(const Module *mod);

  const IDesign *design_;
  const Connection &conn_;
  ostream &os_;
  vector<Module *> ordered_modules_;
  map<const IModule *, Module *> modules_;
  unique_ptr<EmbeddedModules> embedded_modules_;
  string shell_module_name_;
  bool with_self_clock_;
  unique_ptr<Names> names_;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_verilog_writer_h_
