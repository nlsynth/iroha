#include "writer/verilog/wire/mux_node.h"

#include "iroha/logging.h"
#include "writer/verilog/ports.h"
#include "writer/verilog/resource.h"
#include "writer/verilog/table.h"
#include "writer/verilog/wire/accessor_info.h"
#include "writer/verilog/wire/wire_set.h"

namespace iroha {
namespace writer {
namespace verilog {
namespace wire {

MuxNode::MuxNode(const WireSet *ws, const AccessorInfo *accessor)
  : ws_(ws), accessor_(accessor) {
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

void MuxNode::WriteMux(ostream &os) {
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
}

void MuxNode::BuildWriteArg(const SignalDescription &arg_desc,
			    const SignalDescription *req_desc,
			    const SignalDescription *notify_desc,
			    const SignalDescription *notify_secondary_desc,
			    ostream &os) {
  os << "  assign " << ws_->ResourceWireName(arg_desc) << " = ";
  // wire names of write_arg, req
  vector<pair<string, string> > pins;
  vector<pair<string, string> > notify_pins;
  vector<pair<string, string> > notify_secondary_pins;
  for (MuxNode *n : children_) {
    const AccessorInfo *ac = n->accessor_;
    AccessorSignal *warg = ac->FindSignal(arg_desc);
    if (warg == nullptr) {
      continue;
    }
    if (req_desc != nullptr) {
      AccessorSignal *rsig = ac->FindSignal(*req_desc);
      if (rsig != nullptr) {
	pins.push_back(make_pair(ws_->AccessorEdgeWireName(*warg),
				 ws_->AccessorEdgeWireName(*rsig)));
      }
    }
    if (notify_desc != nullptr) {
      AccessorSignal *nsig = ac->FindSignal(*notify_desc);
      if (nsig != nullptr) {
	notify_pins.push_back(make_pair(ws_->AccessorEdgeWireName(*warg),
					ws_->AccessorEdgeWireName(*nsig)));
      }
    }
    if (notify_secondary_desc != nullptr) {
      AccessorSignal *nsig = ac->FindSignal(*notify_secondary_desc);
      if (nsig != nullptr) {
	notify_secondary_pins.push_back(make_pair(ws_->AccessorEdgeWireName(*warg),
						  ws_->AccessorEdgeWireName(*nsig)));
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
  string rwire = ws_->ResourceWireName(arg_desc);
  for (MuxNode *n : children_) {
    const AccessorInfo *ac = n->accessor_;
    AccessorSignal *rsig = ac->FindSignal(arg_desc);
    if (rsig == nullptr) {
      continue;
    }
    os << "  assign " << ws_->AccessorEdgeWireName(*rsig)
       << " = " << rwire << ";\n";
  }
}

void MuxNode::BuildNotifyParent(const SignalDescription &desc, ostream &os) {
  vector<string> wires;
  for (MuxNode *n : children_) {
    const AccessorInfo *ac = n->accessor_;
    AccessorSignal *sig = ac->FindSignal(desc);
    if (sig == nullptr) {
      continue;
    }
    wires.push_back(ws_->AccessorEdgeWireName(*sig));
  }
  os << "  assign " << ws_->ResourceWireName(desc) << " = ";
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
  vector<const AccessorInfo*> handshake_accessors;
  for (MuxNode *n : children_) {
    const AccessorInfo *ac = n->accessor_;
    AccessorSignal *rsig = ac->FindSignal(req_desc);
    if (rsig == nullptr) {
      continue;
    }
    AccessorSignal *asig = ac->FindSignal(ack_desc);
    CHECK(asig != nullptr);
    handshake_accessors.push_back(ac);
  }
  // Registered req.
  BuildRegisteredReq(req_desc, handshake_accessors, os);
  // Req.
  vector<string> req_sigs;
  for (auto *ac : handshake_accessors) {
    AccessorSignal *rsig = ac->FindSignal(req_desc);
    req_sigs.push_back(ws_->AccessorEdgeWireName(*rsig));
  }
  os << "  assign " << ws_->ResourceWireName(req_desc) << " = "
     << Util::Join(req_sigs, " | ") << ";\n";
  // Accessor Acks.
  BuildAccessorAck(req_desc, ack_desc, handshake_accessors, os);
}

void MuxNode::BuildRegisteredReq(const SignalDescription &req_desc,
				 vector<const AccessorInfo *> &handshake_accessors,
				 ostream &os) {
  string initial, body;
  for (auto *ac : handshake_accessors) {
    AccessorSignal *rsig = ac->FindSignal(req_desc);
    string reg = ws_->AccessorWireNameWithReg(*rsig);
    string wire = ws_->AccessorEdgeWireName(*rsig);
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
			       vector<const AccessorInfo *> &handshake_accessors,
			       ostream &os) {
  string resource_ack = ws_->ResourceWireName(ack_desc);
  vector<string> high_reqs;
  for (auto *ac : handshake_accessors) {
    AccessorSignal *asig = ac->FindSignal(ack_desc);
    AccessorSignal *rsig = ac->FindSignal(req_desc);
    string req = ws_->AccessorEdgeWireName(*rsig);
    // ack = resource_ack & req & !(req from higher accessors).
    os << "  assign " << ws_->AccessorEdgeWireName(*asig) << " = "
       << resource_ack << " & " << req;
    if (high_reqs.size() > 0) {
      os << " & !(" <<  Util::Join(high_reqs, " | ") << ")";
    }
    os << ";\n";
    high_reqs.push_back(ws_->AccessorWireNameWithReg(*rsig));
  }
}

}  // namespace wire
}  // namespace verilog
}  // namespace writer
}  // namespace iroha
