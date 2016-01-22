// -*- C++ -*-
#ifndef _writer_verilog_verilog_writer_h_
#define _writer_verilog_verilog_writer_h_

#include "iroha/common.h"

#include <map>

namespace iroha {
namespace writer {

class Connection;

namespace verilog {

class Embed;
class Module;

class VerilogWriter {
public:
  VerilogWriter(const IDesign *design, const Connection &conn, ostream &os);
  ~VerilogWriter();

  void Write();

private:
  void BuildModules(const IModule *imod);
  void BuildHierarchy();

  const IDesign *design_;
  const Connection &conn_;
  ostream &os_;
  vector<Module *> ordered_modules_;
  map<const IModule *, Module *> modules_;
  unique_ptr<Embed> embed_;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_verilog_writer_h_

