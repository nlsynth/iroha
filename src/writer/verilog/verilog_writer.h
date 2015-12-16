// -*- C++ -*-
#ifndef _writer_writer_verilog_writer_h_
#define _writer_writer_verilog_writer_h_

#include "iroha/common.h"

namespace iroha {

class IDesign;

namespace verilog {

class VerilogWriter {
public:
  VerilogWriter(const IDesign *design, ostream &os);

  void Write();
private:
  const IDesign *design_;
  ostream &os_;
};

}  // namespace verilog
}  // namespace iroha

#endif  // _writer_writer_verilog_writer_h_

