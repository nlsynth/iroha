// -*- C++ -*-
#ifndef _writer_verilog_indent_h_
#define _writer_verilog_indent_h_

#include "writer/verilog/common.h"
namespace iroha {
namespace writer {
namespace verilog {

class Indent {
 public:
  Indent(const string &s);

  string DoIndent();

 private:
  const string &s_;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_indent_h_
