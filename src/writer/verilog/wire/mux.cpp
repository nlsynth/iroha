#include "writer/verilog/wire/mux.h"

#include "iroha/logging.h"
#include "writer/verilog/ports.h"
#include "writer/verilog/resource.h"
#include "writer/verilog/table.h"
#include "writer/verilog/wire/accessor_info.h"
#include "writer/verilog/wire/wire_set.h"

// edge wire -> arbitration/handshake logic -> resource wire

namespace iroha {
namespace writer {
namespace verilog {
namespace wire {

Mux::Mux(const WireSet *ws) : ws_(ws) {
  ports_.reset(new Ports);
}

Mux::~Mux() {
}

void Mux::Write(const WireSet *ws, ostream &os) {
  Mux mux(ws);
  mux.DoWrite(os);
}

void Mux::DoWrite(ostream &os) {
  const auto &acs = ws_->GetAccessors();
  for (auto *ac : acs) {
    auto &asigs = ac->GetSignals();
    for (auto &asig : asigs) {
      string s = ws_->AccessorEdgeWireName(*asig);
      int w = asig->sig_desc_->width_;
      if (asig->sig_desc_->IsUpstream()) {
	ports_->AddPort(s, Port::INPUT, w);
      } else {
	ports_->AddPort(s, Port::OUTPUT_WIRE, w);
      }
    }
  }
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

  os << "\nmodule " << ws_->GetMuxName() << "(\n"
     << "  input clk, input rst";
  if (sigs.size() > 0) {
    os << ",";
  }
  ports_->Output(Ports::PORT_MODULE_HEAD, os);
  os << ");\n";

  WriteMux(os);

  os << "endmodule // " << ws_->GetMuxName() << "\n";
}

void Mux::WriteMux(ostream &os) {
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

void Mux::BuildWriteArg(const SignalDescription &arg_desc,
			const SignalDescription *req_desc,
			const SignalDescription *notify_desc,
			const SignalDescription *notify_secondary_desc,
			ostream &os) {
  os << "  assign " << ws_->ResourceWireName(arg_desc) << " = ";
  // wire names of write_arg, req
  vector<pair<string, string> > pins;
  vector<pair<string, string> > notify_pins;
  vector<pair<string, string> > notify_secondary_pins;
  const auto &acs = ws_->GetAccessors();
  for (AccessorInfo *ac : acs) {
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

void Mux::BuildReadArg(const SignalDescription &arg_desc,
		       ostream &os) {
  string rwire = ws_->ResourceWireName(arg_desc);
  const auto &acs = ws_->GetAccessors();
  for (AccessorInfo *ac : acs) {
    AccessorSignal *rsig = ac->FindSignal(arg_desc);
    if (rsig == nullptr) {
      continue;
    }
    os << "  assign " << ws_->AccessorEdgeWireName(*rsig)
       << " = " << rwire << ";\n";
  }
}

void Mux::BuildNotifyParent(const SignalDescription &desc, ostream &os) {
  vector<string> wires;
  const auto &acs = ws_->GetAccessors();
  for (AccessorInfo *ac : acs) {
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

void Mux::BuildNotifyAccessor(const SignalDescription &desc, ostream &os) {
  BuildReadArg(desc, os);
}

void Mux::BuildArbitration(const SignalDescription &req_desc,
			   const SignalDescription &ack_desc,
			   ostream &os) {
  vector<AccessorInfo*> handshake_accessors;
  const auto &acs = ws_->GetAccessors();
  for (auto *ac : acs) {
    AccessorSignal *rsig = ac->FindSignal(req_desc);
    if (rsig == nullptr) {
      continue;
    }
    AccessorSignal *asig = ac->FindSignal(ack_desc);
    CHECK(asig != nullptr);
    handshake_accessors.push_back(ac);
  }
  os << "  // Arbitration and handshake - " << ws_->GetResourceName() << "\n";
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

void Mux::BuildRegisteredReq(const SignalDescription &req_desc,
			     vector<AccessorInfo *> &handshake_accessors,
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
  os << "  always @(posedge clk) begin\n"
     << "    if (rst) begin\n";
  os << initial;
  os << "    end else begin\n";
  os << body;
  os << "    end\n"
     << "  end\n";
}

void Mux::BuildAccessorAck(const SignalDescription &req_desc,
			   const SignalDescription &ack_desc,
			   vector<AccessorInfo *> &handshake_accessors,
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
