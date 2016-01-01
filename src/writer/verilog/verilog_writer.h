// -*- C++ -*-
#ifndef _writer_verilog_verilog_writer_h_
#define _writer_verilog_verilog_writer_h_

#include "iroha/common.h"

#include <map>

namespace iroha {

class IDesign;

namespace verilog {

class Embed;
class Module;

class VerilogWriter {
public:
  VerilogWriter(const IDesign *design, ostream &os);
  ~VerilogWriter();

  void Write();

private:
  void BuildModules(const IModule *imod);
  void BuildHierarchy();

  const IDesign *design_;
  ostream &os_;
  vector<Module *> ordered_modules_;
  map<const IModule *, Module *> modules_;
  unique_ptr<Embed> embed_;
};

}  // namespace verilog
}  // namespace iroha

#endif  // _writer_verilog_verilog_writer_h_

