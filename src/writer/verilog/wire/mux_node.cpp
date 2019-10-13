#include "writer/verilog/wire/mux_node.h"

#include "iroha/logging.h"
#include "writer/verilog/ports.h"
#include "writer/verilog/resource.h"
#include "writer/verilog/table.h"
#include "writer/verilog/wire/accessor_info.h"
#include "writer/verilog/wire/mux.h"
#include "writer/verilog/wire/wire_set.h"

// Up-stream <--> Down-stream
//
// Basic structure:
//   NodeWire <---Mux+--- c0.NodeWire()
//                   +--- c1.NodeWire()
//                        ..
//
// Staged:
//   wdata:
//     NodeWire = NodeWireWithReg
//     NodeWireWithReg <= aribiter(c0.NodeWire(), c1.NodeWire(), ...)
//   rdata, notify accessor:
//     NodeWireWithReg <= NodeWire
//     c0.NodeWire = NodeWireWithReg
//     c1.NodeWire = NodeWireWithReg
//     ...
//   notify parent:
//     NodeWire = NodeWireWithReg
//     NodeWireWithReg <= |c0.NodeWire(), c1.NodeWire, ...
//   req:
//     NodeWire = NodeWireWithReg
//     NodeWireWithReg <= |c0.NodeWire(), c1.NodeWire, ...
//   ack:
//     NodeWireWithReg <= NodeWire
//     c0.NodeWire = arbiter(NodeWireWithReg)
//     c1.NodeWire = arbiter(NodeWireWithReg)

namespace iroha {
namespace writer {
namespace verilog {
namespace wire {

MuxNode::MuxNode(const Mux *mux, int id, const AccessorInfo *accessor)
  : mux_(mux), id_(id), ws_(mux->GetWireSet()), accessor_(accessor) {
}

bool MuxNode::IsRoot() const {
  return (mux_->GetRootNode() == this);
}

bool MuxNode::IsLeaf() const {
  return (accessor_ != nullptr);
}

bool MuxNode::IsStaged() const {
  // WIP.
  return false; // !(IsRoot() || IsLeaf());
}

void MuxNode::WriteIOWire(Ports *ports, ostream &os) {
  for (MuxNode *cn : children_) {
    cn->WriteIOWire(ports, os);
  }
  // Root or leaf nodes have external wires.
  if (IsRoot()) {
    auto sigs = ws_->GetSignals();
    for (auto &sig : sigs) {
      string n = NodeWireName(*sig);
      int w = sig->width_;
      if (sig->IsUpstream()) {
	ports->AddPort(n, Port::OUTPUT_WIRE, w);
      } else {
	ports->AddPort(n, Port::INPUT, w);
      }
    }
  }
  if (IsLeaf()) {
    const auto &asigs = accessor_->GetSignals();
    for (auto &asig : asigs) {
      string s = NodeWireName(*(asig->sig_desc_));
      int w = asig->sig_desc_->width_;
      if (asig->sig_desc_->IsUpstream()) {
	ports->AddPort(s, Port::INPUT, w);
      } else {
	ports->AddPort(s, Port::OUTPUT_WIRE, w);
      }
    }
  }
}

void MuxNode::WriteMux(ostream &os) {
  if (IsLeaf()) {
    return;
  }
  os << "\n  // node:" << id_ << " " << children_.size() << " child nodes";
  if (IsStaged()) {
    os << " - staged";
  }
  os << "\n";
  auto sigs = ws_->GetSignals();
  SignalDescription *req_desc = nullptr;
  SignalDescription *ack_desc = nullptr;
  SignalDescription *notify_desc = nullptr;
  SignalDescription *notify_secondary_desc = nullptr;
  for (SignalDescription *desc : sigs) {
    if (desc->type_ == AccessorSignalType::ACCESSOR_REQ) {
      req_desc = desc;
    }
    if (desc->type_ == AccessorSignalType::ACCESSOR_ACK) {
      ack_desc = desc;
    }
    if (desc->type_ == AccessorSignalType::ACCESSOR_NOTIFY_PARENT) {
      notify_desc = desc;
    }
    if (desc->type_ == AccessorSignalType::ACCESSOR_NOTIFY_PARENT_SECONDARY) {
      notify_secondary_desc = desc;
    }
  }
  if (req_desc != nullptr && ack_desc != nullptr) {
    BuildArbitration(*req_desc, *ack_desc, os);
  }
  for (SignalDescription *desc : sigs) {
    switch (desc->type_) {
    case AccessorSignalType::ACCESSOR_WRITE_ARG:
      BuildWriteArg(*desc, req_desc, notify_desc, notify_secondary_desc, os);
      break;
    case AccessorSignalType::ACCESSOR_READ_ARG:
      BuildReadArg(*desc, os);
      break;
    case AccessorSignalType::ACCESSOR_NOTIFY_PARENT:
    case AccessorSignalType::ACCESSOR_NOTIFY_PARENT_SECONDARY:
      BuildNotifyParent(*desc, os);
      break;
    case AccessorSignalType::ACCESSOR_NOTIFY_ACCESSOR:
      BuildNotifyAccessor(*desc, os);
      break;
    default:
      break;
    }
  }
  if (IsStaged()) {
    WriteStage(os);
  }
  for (MuxNode *cn : children_) {
    cn->WriteMux(os);
  }
}

void MuxNode::WriteStage(ostream &os) {
  const Table &tab = ws_->GetResource().GetTable();
  os << "\n  // stage\n" << as_.str();
  tab.WriteAlwaysBlockHead(os);
  os << is_.str();
  tab.WriteAlwaysBlockMiddle(os);
  os << ss_.str();
  tab.WriteAlwaysBlockTail(os);
}

void MuxNode::WriteDecls(ostream &os) {
  if (IsLeaf()) {
    return;
  }
  for (MuxNode *cn : children_) {
    cn->WriteDecls(os);
  }
  auto sigs = ws_->GetSignals();
  for (SignalDescription *desc : sigs) {
    if (!IsRoot()) {
      os << "  wire " << Table::WidthSpec(desc->width_)
	 << NodeWireName(*desc) << ";\n";
    }
    if (IsStaged()) {
      os << "  reg " << Table::WidthSpec(desc->width_)
	 << NodeWireNameWithReg(*desc) << ";\n";
    }
  }
  if (IsStaged()) {
    os << "  reg [1:0] " << HandShakeState() << ";\n";
  }
}

void MuxNode::BuildWriteArg(const SignalDescription &arg_desc,
			    const SignalDescription *req_desc,
			    const SignalDescription *notify_desc,
			    const SignalDescription *notify_secondary_desc,
			    ostream &os) {
  // wire names of write_arg, req
  vector<pair<string, string> > pins;
  vector<pair<string, string> > notify_pins;
  vector<pair<string, string> > notify_secondary_pins;
  for (MuxNode *n : children_) {
    if (n->IsLeaf()) {
      AccessorSignal *warg = n->accessor_->FindSignal(arg_desc);
      if (warg == nullptr) {
	continue;
      }
    }
    bool isInternal = !n->IsLeaf();
    const AccessorInfo *ac = n->accessor_;
    if (req_desc != nullptr) {
      if (isInternal || ac->FindSignal(*req_desc) != nullptr) {
	pins.push_back(make_pair(n->NodeWireName(arg_desc),
				 n->NodeWireName(*req_desc)));
      }
    }
    if (notify_desc != nullptr) {
      if (isInternal || ac->FindSignal(*notify_desc) != nullptr) {
	notify_pins.push_back(make_pair(n->NodeWireName(arg_desc),
					n->NodeWireName(*notify_desc)));
      }
    }
    if (notify_secondary_desc != nullptr) {
      if (isInternal || ac->FindSignal(*notify_secondary_desc) != nullptr) {
	notify_secondary_pins.push_back(make_pair(n->NodeWireName(arg_desc),
						  n->NodeWireName(*notify_secondary_desc)));
      }
    }
  }
  // Order: Req based writes -> Notify writes -> Secondary notify wires.
  // MEMO: Consider "(Notify | Secondary notify)" instead of separated list.
  for (auto &p : notify_pins) {
    pins.push_back(p);
  }
  for (auto &p : notify_secondary_pins) {
    pins.push_back(p);
  }
  // reverse order.
  string s;
  for (int i = pins.size() - 1; i >= 0; --i) {
    if (s.empty()) {
      s = pins[i].first;
    } else {
      s = pins[i].second + " ? " + pins[i].first + " : (" + s + ")";
    }
  }
  if (IsStaged()) {
    os << "  wire " << Table::WidthSpec(arg_desc.width_)
       << NodeWireNameWithSrc(arg_desc) << ";\n";
  }
  os << "  assign ";
  if (IsStaged()) {
    os << NodeWireNameWithSrc(arg_desc);
    ss_ << "      " << NodeWireNameWithReg(arg_desc) << " <= "
	<< NodeWireNameWithSrc(arg_desc) << ";\n";
    as_ << "  assign " << NodeWireName(arg_desc) << " = "
	<< NodeWireNameWithReg(arg_desc) << ";\n";
    is_ << "      " << NodeWireNameWithReg(arg_desc) << " <= 0;\n";
  } else {
    os << NodeWireName(arg_desc);
  }
  os << " = ";
  if (s.empty()) {
    s = "0";
  }
  os << s << ";\n";
}

void MuxNode::BuildReadArg(const SignalDescription &arg_desc,
			   ostream &os) {
  string rwire = NodeWireName(arg_desc);
  if (IsStaged()) {
    ss_ << "      " << NodeWireNameWithReg(arg_desc)
	<< " <= " << NodeWireName(arg_desc) << ";\n";
    is_ << "      " << NodeWireNameWithReg(arg_desc)
	<< " <= 0;\n";
    rwire = NodeWireNameWithReg(arg_desc);
  }
  for (MuxNode *n : children_) {
    if (n->IsLeaf()) {
      const AccessorInfo *ac = n->accessor_;
      AccessorSignal *rsig = ac->FindSignal(arg_desc);
      if (rsig == nullptr) {
	continue;
      }
    }
    os << "  assign " << n->NodeWireName(arg_desc)
       << " = " << rwire << ";\n";
  }
}

void MuxNode::BuildNotifyParent(const SignalDescription &desc, ostream &os) {
  vector<string> wires;
  for (MuxNode *n : children_) {
    if (n->IsLeaf()) {
      const AccessorInfo *ac = n->accessor_;
      AccessorSignal *sig = ac->FindSignal(desc);
      if (sig == nullptr) {
	continue;
      }
    }
    wires.push_back(n->NodeWireName(desc));
  }
  os << "  assign ";
  if (IsStaged()) {
    os << NodeWireNameWithSrc(desc);
    ss_ << "      " << NodeWireNameWithReg(desc) << " <= "
	<< NodeWireNameWithSrc(desc) << ";\n";
    as_ << "  assign " << NodeWireName(desc) << " = "
	<< NodeWireNameWithReg(desc) << ";\n";
    is_ << "      " << NodeWireNameWithReg(desc) << " <= 0;\n";
  } else {
    os << NodeWireName(desc);
  }
  os << " = ";
  if (wires.size() == 0) {
    os << "0";
  } else {
    os << Util::Join(wires, " | ");
  }
  os << ";\n";
}

void MuxNode::BuildNotifyAccessor(const SignalDescription &desc, ostream &os) {
  BuildReadArg(desc, os);
}

void MuxNode::BuildArbitration(const SignalDescription &req_desc,
			       const SignalDescription &ack_desc,
			       ostream &os) {
  vector<const MuxNode *> handshake_nodes;
  for (MuxNode *n : children_) {
    if (n->IsLeaf()) {
      const AccessorInfo *ac = n->accessor_;
      AccessorSignal *rsig = ac->FindSignal(req_desc);
      if (rsig == nullptr) {
	continue;
      }
      AccessorSignal *asig = ac->FindSignal(ack_desc);
      CHECK(asig != nullptr);
    }
    handshake_nodes.push_back(n);
  }
  // Registered req.
  BuildRegisteredReq(req_desc, handshake_nodes, os);
  // Req.
  vector<string> req_sigs;
  for (auto *n : handshake_nodes) {
    req_sigs.push_back(n->NodeWireName(req_desc));
  }
  os << "  assign ";
  if (IsStaged()) {
    os << NodeWireNameWithSrc(req_desc);
    as_ << "  assign " << NodeWireName(req_desc) << " = "
	<< NodeWireNameWithReg(req_desc) << ";\n";
    is_ << "      " << NodeWireNameWithReg(req_desc) << " <= 0;\n";
  } else {
    os << NodeWireName(req_desc);
  }
  os << " = ";
  if (req_sigs.size() > 0) {
    os << Util::Join(req_sigs, " | ");
  } else {
    os << "0";
  }
  os << ";\n";
  // Accessor Acks.
  BuildAccessorAck(req_desc, ack_desc, handshake_nodes, os);
  // Handshake.
  if (IsStaged()) {
    BuildStageHandShake(req_desc, ack_desc);
  }
}

void MuxNode::BuildStageHandShake(const SignalDescription &req_desc,
				  const SignalDescription &ack_desc) {
  string st = HandShakeState();
  is_ << "      " << st << " <= 0;\n";
  ss_ << "      case (" << st << ")\n"
      << "        0: begin\n"
      << "          if (" << NodeWireNameWithSrc(req_desc) << ") begin\n"
      << "            " << NodeWireNameWithReg(req_desc) << " <= 1;\n"
      << "            " << st << " <= 1;\n"
      << "          end\n"
      << "        end\n"
      << "        1: begin\n"
      << "          if (" << NodeWireName(ack_desc) << ") begin\n"
      << "            " << NodeWireNameWithReg(req_desc) << " <= 0;\n"
      << "            " << NodeWireNameWithReg(ack_desc) << " <= 1;\n"
      << "            " << st << " <= 2;\n"
      << "          end\n"
      << "        end\n"
      << "        2: begin\n"
      << "            " << NodeWireNameWithReg(ack_desc) << " <= 0;\n"
      << "            " << st << " <= 3;\n"
      << "        end\n"
      << "        3: begin\n"
      << "          if (!" << NodeWireNameWithSrc(req_desc) << ") begin\n"
      << "            " << st << " <= 0;\n"
      << "          end\n"
      << "        end\n"
      << "      endcase\n";
}

void MuxNode::BuildRegisteredReq(const SignalDescription &req_desc,
				 vector<const MuxNode *> &handshake_nodes,
				 ostream &os) {
  string initial, body;
  for (auto *n : handshake_nodes) {
    // Used to generate ack signals.
    string prev_reg = n->NodeWireNameWithPrev(req_desc);
    string wire = n->NodeWireName(req_desc);
    os << "  reg " << prev_reg << ";\n";
    initial += "      " + prev_reg + " <= 0;\n";
    body += "      " + prev_reg + " <= " + wire + ";\n";
  }
  const Table &tab = ws_->GetResource().GetTable();
  tab.WriteAlwaysBlockHead(os);
  os << initial;
  tab.WriteAlwaysBlockMiddle(os);
  os << body;
  tab.WriteAlwaysBlockTail(os);
}

void MuxNode::BuildAccessorAck(const SignalDescription &req_desc,
			       const SignalDescription &ack_desc,
			       vector<const MuxNode *> &handshake_nodes,
			       ostream &os) {
  string resource_ack;
  if (IsStaged()) {
    resource_ack = NodeWireNameWithReg(ack_desc);
    is_ << "      " << NodeWireNameWithReg(ack_desc) << " <= 0;\n";
  } else {
    resource_ack = NodeWireName(ack_desc);
  }
  vector<string> high_reqs;
  for (auto *n : handshake_nodes) {
    string req = n->NodeWireName(req_desc);
    // ack = resource_ack & req & !(req from higher accessors).
    os << "  assign "
       << n->NodeWireName(ack_desc)
       << " = " << resource_ack << " & " << req;
    if (high_reqs.size() > 0) {
      os << " & !(" <<  Util::Join(high_reqs, " | ") << ")";
    }
    os << ";\n";
    high_reqs.push_back(n->NodeWireNameWithPrev(req_desc));
  }
}

string MuxNode::NodeWireName(const SignalDescription &desc) const {
  if (IsLeaf()) {
    AccessorSignal *sig = accessor_->FindSignal(desc);
    return ws_->AccessorEdgeWireName(*sig);
  }
  if (IsRoot()) {
    return ws_->ResourceWireName(desc);
  }
  return "node" + Util::Itoa(id_) + "_" + desc.name_;
}

string MuxNode::NodeWireNameWithPrev(const SignalDescription &desc) const {
  return NodeWireName(desc) + "_prev";
}

string MuxNode::NodeWireNameWithReg(const SignalDescription &desc) const {
  return NodeWireName(desc) + "_reg";
}

string MuxNode::NodeWireNameWithSrc(const SignalDescription &desc) const {
  return NodeWireName(desc) + "_src";
}

string MuxNode::HandShakeState() const {
  return "st" + Util::Itoa(id_);
}

}  // namespace wire
}  // namespace verilog
}  // namespace writer
}  // namespace iroha
