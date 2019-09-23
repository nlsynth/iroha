#include "writer/verilog/wire/mux.h"

#include "iroha/logging.h"
#include "writer/verilog/ports.h"
#include "writer/verilog/table.h"
#include "writer/verilog/resource.h"
#include "writer/verilog/wire/accessor_info.h"
#include "writer/verilog/wire/mux_node.h"
#include "writer/verilog/wire/wire_set.h"

// edge wire -> arbitration/handshake logic -> resource wire

namespace iroha {
namespace writer {
namespace verilog {
namespace wire {

Mux::Mux(const WireSet *ws) : ws_(ws), root_node_(nullptr) {
  ports_.reset(new Ports);
  const Table &tab = ws_->GetResource().GetTable();
  Ports *pp = tab.GetPorts();
  ports_->AddPort(pp->GetClk(), Port::INPUT_CLK, 0);
  ports_->AddPort(pp->GetReset(), Port::INPUT_RESET, 0);
  root_node_ = BuildNodes(ws_->GetAccessors());
}

Mux::~Mux() {
  DeleteNode(root_node_);
}

void Mux::Write(const WireSet *ws, ostream &os) {
  Mux mux(ws);
  mux.DoWrite(os);
}

void Mux::DoWrite(ostream &os) {
  root_node_->WriteIOWire(ports_.get(), os);
  auto sigs = ws_->GetSignals();
  for (auto *sig : sigs) {
    string n = ws_->ResourceWireName(*sig);
    int w = sig->width_;
    if (sig->IsUpstream()) {
      ports_->AddPort(n, Port::OUTPUT_WIRE, w);
    } else {
      ports_->AddPort(n, Port::INPUT, w);
    }
  }

  os << "\nmodule " << ws_->GetMuxName() << "(";
  ports_->Output(Ports::PORT_MODULE_HEAD, os);
  os << ");\n";

  root_node_->WriteMux(os);

  os << "endmodule // " << ws_->GetMuxName() << "\n";
}

MuxNode *Mux::BuildNodes(const vector<AccessorInfo *> &acs) {
  MuxNode *root = new MuxNode(ws_, nullptr);
  for (auto *ac : acs) {
    MuxNode *node = new MuxNode(ws_, ac);
    root->children_.push_back(node);
  }
  return root;
}

void Mux::DeleteNode(MuxNode *node) {
  if (node == nullptr) {
    return;
  }
  for (MuxNode *c : node->children_) {
    DeleteNode(c);
  }
  delete node;
}

}  // namespace wire
}  // namespace verilog
}  // namespace writer
}  // namespace iroha
