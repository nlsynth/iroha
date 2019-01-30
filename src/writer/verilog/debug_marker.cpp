#include "writer/verilog/debug_marker.h"

namespace iroha {
namespace writer {
namespace verilog {

bool DebugMarker::enabled_;

string DebugMarker::Output(const char *fn, int ln, const char *msg) {
  if (!enabled_) {
    return "";
  }
  string m = string(fn) + ":" + Util::Itoa(ln);
  if (msg != nullptr) {
    m = string(msg) + " " + m + "\n";
  }
  return m;
}

void DebugMarker::SetEnable(bool b) {
  enabled_ = b;
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
