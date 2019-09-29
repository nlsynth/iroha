#include "writer/verilog/wire/mux.h"

#include "iroha/logging.h"
#include "writer/verilog/ports.h"
#include "writer/verilog/table.h"
#include "writer/verilog/resource.h"
#include "writer/verilog/wire/accessor_info.h"
#include "writer/verilog/wire/mux_node.h"
#include "writer/verilog/wire/wire_set.h"

// (accessor*s* ... ->)
// edge wire -> arbitration/handshake logic -> resource wire
// (... -> parent resource)

// // a mux module will look like this.
// module mux_name(accessor 1 wires, accessor 2 wires,,, resource wire);
//   // arbitration logic
//   ... for each MuxNode ...
// endmodule

namespace iroha {
namespace writer {
namespace verilog {
namespace wire {

Mux::Mux(const WireSet *ws) : ws_(ws), root_node_(nullptr), num_nodes_(0) {
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

const WireSet *Mux::GetWireSet() const {
  return ws_;
}

MuxNode *Mux::GetRootNode() const {
  return root_node_;
}

void Mux::DoWrite(ostream &os) {
  root_node_->WriteIOWire(ports_.get(), os);

  os << "\nmodule " << ws_->GetMuxName() << "(";
  ports_->Output(Ports::PORT_MODULE_HEAD, os);
  os << ");\n";

  root_node_->WriteMux(os);

  os << "endmodule // " << ws_->GetMuxName() << "\n";
}

MuxNode *Mux::BuildNodes(const vector<AccessorInfo *> &acs) {
  MuxNode *root = new MuxNode(this, 0, nullptr);
  for (auto *ac : acs) {
    MuxNode *node = new MuxNode(this, 0, ac);
    root->children_.push_back(node);
  }
  BalanceNode(root);
  return root;
}

void Mux::BalanceNode(MuxNode *node) {
  int s = node->children_.size();
  int f = MaxFanOut();
  if (s < f) {
    return;
  }
  vector<MuxNode *> new_nodes;
  // Set up f nodes.
  for (int i = 0; i < f; ++i) {
    num_nodes_++;
    MuxNode *c = new MuxNode(this, num_nodes_, nullptr);
    new_nodes.push_back(c);
  }
  int j = 0;
  for (int i = 0; i < s; ++i) {
    // distribute s original children to f nodes.
    new_nodes[j]->children_.push_back(node->children_[i]);
    ++j;
    if (j == f) {
      j = 0;
    }
  }
  node->children_ = new_nodes;
  for (MuxNode *c : node->children_) {
    BalanceNode(c);
  }
}

int Mux::MaxFanOut() {
  // WIP.
  return 10000;
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
