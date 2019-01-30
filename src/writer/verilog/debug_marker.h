// -*- C++ -*-
#ifndef _writer_verilog_debug_marker_h_
#define _writer_verilog_debug_marker_h_

#include "writer/verilog/common.h"

namespace iroha {
namespace writer {
namespace verilog {

#define DEBUG_MARKER() (DebugMarker::Output(__FILE__, __LINE__, nullptr))
#define DEBUG_MESSAGE(msg) (DebugMarker::Output(__FILE__, __LINE__, msg))

class DebugMarker {
public:
  static void SetEnable(bool b);
  static string Output(const char *fn, int ln, const char *msg);

private:
  static bool enabled_;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_debug_marker_h_
