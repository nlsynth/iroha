// -*- C++ -*-
#ifndef _writer_verilog_wire_mux_h_
#define _writer_verilog_wire_mux_h_

#include "writer/verilog/common.h"

namespace iroha {
namespace writer {
namespace verilog {
namespace wire {

class Mux {
public:
  static void Write(const WireSet *ws);
};

}  // namespace wire
}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_wire_mux_h_
