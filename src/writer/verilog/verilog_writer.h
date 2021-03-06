// -*- C++ -*-
#ifndef _writer_verilog_verilog_writer_h_
#define _writer_verilog_verilog_writer_h_

#include <map>
#include <sstream>

#include "writer/verilog/common.h"

namespace iroha {
namespace writer {
namespace verilog {

class VerilogWriter {
 public:
  VerilogWriter(const IDesign *design, const Connection &conn,
                const string &flavor, bool debug, ostream &os);
  ~VerilogWriter();

  bool Write();
  void SetShellModuleName(const string &n, bool with_self_contained,
                          bool output_vcd);
  Module *GetByIModule(const IModule *mod) const;
  bool GetResetPolarity() const;

 private:
  void PrepareModulesRec(const IModule *imod);
  void BuildModules(const IModule *imod);
  void BuildChildModuleSection();
  void WriteShellModule(const Module *mod);
  void WriteSelfClockGenerator(const Module *root_mod);
  void WriteSelfClockConnection(const Module *root_mod);
  void ResolveResetPolarity(const IModule *root);

  const IDesign *design_;
  const Connection &conn_;
  const string &flavor_;
  bool debug_;
  stringstream os_;
  ostream &final_os_;
  vector<Module *> ordered_modules_;
  map<const IModule *, Module *> modules_;
  unique_ptr<EmbeddedModules> embedded_modules_;
  string shell_module_name_;
  bool with_self_contained_;
  bool output_vcd_;
  unique_ptr<Names> names_root_;
  bool reset_polarity_;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_verilog_writer_h_
