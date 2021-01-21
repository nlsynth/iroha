// -*- C++ -*-
#ifndef _writer_verilog_indent_h_
#define _writer_verilog_indent_h_

#include <map>

#include "writer/verilog/common.h"

namespace iroha {
namespace writer {
namespace verilog {

class Indent {
 public:
  Indent(const string &s);

  string DoIndent();

 private:
  string StripSpace(const string &s);
  string AddIndent(const string &s, int level);
  int PreUpdateLevel(const string &line, int level);
  int PostUpdateLevel(const string &line, int level);

  const string &s_;
  map<int, string> spc_;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_indent_h_
