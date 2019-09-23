#include "writer/verilog/wire/mux_node.h"

#include "writer/verilog/ports.h"
#include "writer/verilog/wire/accessor_info.h"
#include "writer/verilog/wire/wire_set.h"

namespace iroha {
namespace writer {
namespace verilog {
namespace wire {

MuxNode::MuxNode(const WireSet *ws) : ws_(ws), accessor_(nullptr) {
}

void MuxNode::WriteIOWire(Ports *ports, ostream &os) {
  for (MuxNode *cn : children_) {
    cn->WriteIOWire(ports, os);
  }
  if (accessor_ == nullptr) {
    return;
  }
  // Writes external wire from a leaf node.
  const auto &asigs = accessor_->GetSignals();
  for (auto &asig : asigs) {
    string s = ws_->AccessorEdgeWireName(*asig);
    int w = asig->sig_desc_->width_;
    if (asig->sig_desc_->IsUpstream()) {
      ports->AddPort(s, Port::INPUT, w);
    } else {
      ports->AddPort(s, Port::OUTPUT_WIRE, w);
    }
  }
}

}  // namespace wire
}  // namespace verilog
}  // namespace writer
}  // namespace iroha
