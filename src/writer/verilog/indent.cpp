#include "writer/verilog/indent.h"

namespace iroha {
namespace writer {
namespace verilog {

Indent::Indent(const string &s) : s_(s) {}

string Indent::DoIndent() { return s_; }

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
