#include "writer/verilog/wire/mux_node.h"

#include "iroha/logging.h"
#include "writer/verilog/ports.h"
#include "writer/verilog/resource.h"
#include "writer/verilog/table.h"
#include "writer/verilog/wire/accessor_info.h"
#include "writer/verilog/wire/mux.h"
#include "writer/verilog/wire/wire_set.h"

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

void MuxNode::WriteIOWire(Ports *ports, ostream &os) {
  for (MuxNode *cn : children_) {
    cn->WriteIOWire(ports, os);
  }
  // Root or leaf nodes have external wires.
  if (IsRoot()) {
    auto sigs = ws_->GetSignals();
    for (auto &sig : sigs) {
      string n = ResourceWireName(*sig);
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
  os << "  // node:" << id_ << " " << children_.size() << " child nodes\n";
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
  for (MuxNode *cn : children_) {
    cn->WriteMux(os);
  }
}

void MuxNode::BuildWriteArg(const SignalDescription &arg_desc,
			    const SignalDescription *req_desc,
			    const SignalDescription *notify_desc,
			    const SignalDescription *notify_secondary_desc,
			    ostream &os) {
  os << "  assign " << ResourceWireName(arg_desc) << " = ";
  // wire names of write_arg, req
  vector<pair<string, string> > pins;
  vector<pair<string, string> > notify_pins;
  vector<pair<string, string> > notify_secondary_pins;
  for (MuxNode *n : children_) {
    if (!n->IsLeaf()) {
      if (req_desc != nullptr) {
	pins.push_back(make_pair(n->NodeWireName(arg_desc),
				 n->NodeWireName(*req_desc)));
      }
      if (notify_desc != nullptr) {
	notify_pins.push_back(make_pair(n->NodeWireName(arg_desc),
					n->NodeWireName(*notify_desc)));
      }
      if (notify_secondary_desc != nullptr) {
	notify_secondary_pins.push_back(make_pair(n->NodeWireName(arg_desc),
						  n->NodeWireName(*notify_secondary_desc)));
      }
      continue;
    }
    const AccessorInfo *ac = n->accessor_;
    AccessorSignal *warg = ac->FindSignal(arg_desc);
    if (warg == nullptr) {
      continue;
    }
    if (req_desc != nullptr) {
      AccessorSignal *rsig = ac->FindSignal(*req_desc);
      if (rsig != nullptr) {
	pins.push_back(make_pair(n->NodeWireName(arg_desc),
				 n->NodeWireName(*req_desc)));
      }
    }
    if (notify_desc != nullptr) {
      AccessorSignal *nsig = ac->FindSignal(*notify_desc);
      if (nsig != nullptr) {
	notify_pins.push_back(make_pair(n->NodeWireName(arg_desc),
					n->NodeWireName(*notify_desc)));
      }
    }
    if (notify_secondary_desc != nullptr) {
      AccessorSignal *nsig = ac->FindSignal(*notify_secondary_desc);
      if (nsig != nullptr) {
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
  os << s << ";\n";
}

void MuxNode::BuildReadArg(const SignalDescription &arg_desc,
			   ostream &os) {
  string rwire = NodeWireName(arg_desc);
  for (MuxNode *n : children_) {
    if (!n->IsLeaf()) {
      os << "  assign " << n->NodeWireName(arg_desc)
	 << " = " << rwire << ";\n";
      continue;
    }
    const AccessorInfo *ac = n->accessor_;
    AccessorSignal *rsig = ac->FindSignal(arg_desc);
    if (rsig == nullptr) {
      continue;
    }
    os << "  assign " << n->NodeWireName(arg_desc)
       << " = " << rwire << ";\n";
  }
}

void MuxNode::BuildNotifyParent(const SignalDescription &desc, ostream &os) {
  vector<string> wires;
  for (MuxNode *n : children_) {
    if (!n->IsLeaf()) {
      wires.push_back(n->NodeWireName(desc));
      continue;
    }
    const AccessorInfo *ac = n->accessor_;
    AccessorSignal *sig = ac->FindSignal(desc);
    if (sig == nullptr) {
      continue;
    }
    wires.push_back(n->NodeWireName(desc));
  }
  os << "  assign " << NodeWireName(desc) << " = ";
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
    if (!n->IsLeaf()) {
      handshake_nodes.push_back(n);
      continue;
    }
    const AccessorInfo *ac = n->accessor_;
    AccessorSignal *rsig = ac->FindSignal(req_desc);
    if (rsig == nullptr) {
      continue;
    }
    AccessorSignal *asig = ac->FindSignal(ack_desc);
    CHECK(asig != nullptr);
    handshake_nodes.push_back(n);
  }
  // Registered req.
  BuildRegisteredReq(req_desc, handshake_nodes, os);
  // Req.
  vector<string> req_sigs;
  for (auto *n : handshake_nodes) {
    if (!n->IsLeaf()) {
      req_sigs.push_back(n->NodeWireName(req_desc));
      continue;
    }
    req_sigs.push_back(n->NodeWireName(req_desc));
  }
  os << "  assign " << ResourceWireName(req_desc) << " = "
     << Util::Join(req_sigs, " | ") << ";\n";
  // Accessor Acks.
  BuildAccessorAck(req_desc, ack_desc, handshake_nodes, os);
}

void MuxNode::BuildRegisteredReq(const SignalDescription &req_desc,
				 vector<const MuxNode *> &handshake_nodes,
				 ostream &os) {
  string initial, body;
  for (auto *n : handshake_nodes) {
    if (!n->IsLeaf()) {
      continue;
    }
    string reg = n->NodeWireNameWithReg(req_desc);
    string wire = n->NodeWireName(req_desc);
    os << "  reg " << reg << ";\n";
    initial += "      " + reg + " <= 0;\n";
    body += "      " + reg + " <= " + wire + ";\n";
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
  string resource_ack = ResourceWireName(ack_desc);
  vector<string> high_reqs;
  for (auto *n : handshake_nodes) {
    if (!n->IsLeaf()) {
      continue;
    }
    string req = n->NodeWireName(req_desc);
    // ack = resource_ack & req & !(req from higher accessors).
    os << "  assign " << n->NodeWireName(ack_desc) << " = "
       << resource_ack << " & " << req;
    if (high_reqs.size() > 0) {
      os << " & !(" <<  Util::Join(high_reqs, " | ") << ")";
    }
    os << ";\n";
    high_reqs.push_back(n->NodeWireNameWithReg(req_desc));
  }
}

string MuxNode::ResourceWireName(const SignalDescription &desc) const {
  return ws_->ResourceWireName(desc);
}

string MuxNode::NodeWireName(const SignalDescription &desc) const {
  if (IsLeaf()) {
    AccessorSignal *sig = accessor_->FindSignal(desc);
    return ws_->AccessorEdgeWireName(*sig);
  }
  if (IsRoot()) {
    return ResourceWireName(desc);
  }
  return "node" + Util::Itoa(id_) + "_" + desc.name_;
}

string MuxNode::NodeWireNameWithReg(const SignalDescription &desc) const {
  return NodeWireName(desc) + "_reg";
}

}  // namespace wire
}  // namespace verilog
}  // namespace writer
}  // namespace iroha
