#include "writer/verilog/indent.h"

#include <iostream>
#include <sstream>

namespace iroha {
namespace writer {
namespace verilog {

Indent::Indent(const string &s) : s_(s) {}

string Indent::DoIndent() {
  stringstream ss(s_);
  string line;
  string res;
  int level = 0;
  while (getline(ss, line)) {
    line = StripSpace(line);
    level = PreUpdateLevel(line, level);
    res += AddIndent(line, level) + "\n";
    level = PostUpdateLevel(line, level);
  }
  return res;
}

string Indent::StripSpace(const string &s) {
  const char *c = s.c_str();
  while (*c == ' ') {
    ++c;
  }
  if (c != s.c_str()) {
    return string(c);
  }
  return s;
}

string Indent::AddIndent(const string &s, int level) {
  if (s.empty()) {
    return s;
  }
  string spc = spc_[level];
  if (spc.empty()) {
    for (int i = 0; i < level; ++i) {
      spc = spc + "  ";
    }
    spc_[level] = spc;
  }
  return spc + s;
}

int Indent::PreUpdateLevel(const string &line, int level) {
  if (line.find("module") == 0 || line.find("endmodule") == 0) {
    return 0;
  }
  if (line.find("end") == 0) {
    return level - 1;
  }
  return level;
}

int Indent::PostUpdateLevel(const string &line, int level) {
  if (line.find("module") == 0) {
    return 1;
  }
  if (line.find("case") == 0) {
    return level + 1;
  }
  if (line.find("begin") == line.size() - 5) {
    return level + 1;
  }
  return level;
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
