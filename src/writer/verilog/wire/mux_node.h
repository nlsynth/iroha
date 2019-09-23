// -*- C++ -*-
#ifndef _writer_verilog_wire_mux_node_h_
#define _writer_verilog_wire_mux_node_h_

#include "writer/verilog/common.h"

namespace iroha {
namespace writer {
namespace verilog {
namespace wire {

// WIP: This will host a tree, but only 1 level for now.
class MuxNode {
public:
  MuxNode(const WireSet *ws);

  void WriteIOWire(Ports *ports, ostream &os);

  const WireSet *ws_;
  const AccessorInfo *accessor_;
  vector<MuxNode *> children_;
};

}  // namespace wire
}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_wire_mux_node_h_
