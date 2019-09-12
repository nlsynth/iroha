#include "writer/verilog/wire/wire_set.h"

#include "iroha/i_design.h"
#include "iroha/stl_util.h"
#include "iroha/logging.h"
#include "writer/module_template.h"
#include "writer/verilog/embed.h"
#include "writer/verilog/module.h"
#include "writer/verilog/resource.h"
#include "writer/verilog/table.h"
#include "writer/verilog/wire/accessor_info.h"
#include "writer/verilog/wire/distance_regs.h"
#include "writer/verilog/wire/inter_module_wire.h"

// Unified interconnect wiring system for
// shared_memory, shared_reg, fifo, task (and study)
// This handles:
//  * Handshake between accessors and a parent.
//  * Arbitration between multiple accessors.
//  * Distance
//  * Hierarchy
//
// Usage:
// WireSet ws(parent, "name");
// AccessorInfo *ac = ws.AddAccessor(writer);
// ac->AddSignal("req", ACCESSOR_REQ);
// ac->AddSignal("ack", ACCESSOR_ACK);
// ws.Build();
//
//
// * (each) Source module.
// {prefix}_{parent}_{accessor}_{name}
// * wire.
// wire: {prefix}_{parent}_{accessor}_{name}_wire
// -- distance, arbiter
// * Sink module.
// parent: {prefix}_{parent}_{name}
//
// accessor resource -> accessor wire -> delay registers
//  -> accesor edge wire
//  -> arbitration/handshake logic
//  -> resource wire -> parent resource
//
// allocated with new WireSet and EmbeddedModules deletes it.


namespace iroha {
namespace writer {
namespace verilog {
namespace wire {

WireSet::WireSet(Resource &res, const string &resource_name)
  : res_(res), resource_name_(resource_name) {
}

WireSet::~WireSet() {
  STLDeleteValues(&accessors_);
  STLDeleteSecondElements(&signal_desc_);
}

AccessorInfo *WireSet::AddAccessor(const IResource *accessor) {
  AccessorInfo *info = new AccessorInfo(this, accessor);
  accessors_.push_back(info);
  return info;
}

void WireSet::Build() {
  SignalDescription *req_desc = nullptr;
  SignalDescription *ack_desc = nullptr;
  SignalDescription *notify_desc = nullptr;
  SignalDescription *notify_secondary_desc = nullptr;
  for (auto it : signal_desc_) {
    SignalDescription *desc = it.second;
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
  const Table &tab = res_.GetTable();
  ostream &rs = tab.ResourceSectionStream();
  string dbg = "  // WireSet: " + resource_name_ + " begin";
  rs << DEBUG_MESSAGE(dbg.c_str());
  for (auto it : signal_desc_) {
    SignalDescription *desc = it.second;
    BuildAccessorWire(*desc);
  }
  BuildResourceWire();
  BuildDistanceRegs();
  if (req_desc != nullptr && ack_desc != nullptr) {
    BuildArbitration(*req_desc, *ack_desc);
  }
  for (auto it : signal_desc_) {
    SignalDescription *desc = it.second;
    switch (desc->type_) {
    case AccessorSignalType::ACCESSOR_WRITE_ARG:
      BuildWriteArg(*desc, req_desc, notify_desc, notify_secondary_desc);
      break;
    case AccessorSignalType::ACCESSOR_READ_ARG:
      BuildReadArg(*desc);
      break;
    case AccessorSignalType::ACCESSOR_NOTIFY_PARENT:
    case AccessorSignalType::ACCESSOR_NOTIFY_PARENT_SECONDARY:
      BuildNotifyParent(*desc);
      break;
    case AccessorSignalType::ACCESSOR_NOTIFY_ACCESSOR:
      BuildNotifyAccessor(*desc);
      break;
    default:
      break;
    }
  }
  rs << DEBUG_MESSAGE("  // WireSet end");
  // Hands over the ownership as well.
  tab.GetEmbeddedModules()->RequestWireMux(this);
}

void WireSet::BuildWriteArg(const SignalDescription &arg_desc,
			    const SignalDescription *req_desc,
			    const SignalDescription *notify_desc,
			    const SignalDescription *notify_secondary_desc) {
  const Table &tab = res_.GetTable();
  ostream &rs = tab.ResourceSectionStream();
  rs << "  assign " << ResourceWireName(arg_desc) << " = ";
  // wire names of write_arg, req
  vector<pair<string, string> > pins;
  vector<pair<string, string> > notify_pins;
  vector<pair<string, string> > notify_secondary_pins;
  for (AccessorInfo *ac : accessors_) {
    AccessorSignal *warg = ac->FindSignal(arg_desc);
    if (warg == nullptr) {
      continue;
    }
    if (req_desc != nullptr) {
      AccessorSignal *rsig = ac->FindSignal(*req_desc);
      if (rsig != nullptr) {
	pins.push_back(make_pair(AccessorEdgeWireName(*warg),
				 AccessorEdgeWireName(*rsig)));
      }
    }
    if (notify_desc != nullptr) {
      AccessorSignal *nsig = ac->FindSignal(*notify_desc);
      if (nsig != nullptr) {
	notify_pins.push_back(make_pair(AccessorEdgeWireName(*warg),
					AccessorEdgeWireName(*nsig)));
      }
    }
    if (notify_secondary_desc != nullptr) {
      AccessorSignal *nsig = ac->FindSignal(*notify_secondary_desc);
      if (nsig != nullptr) {
	notify_secondary_pins.push_back(make_pair(AccessorEdgeWireName(*warg),
						  AccessorEdgeWireName(*nsig)));
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
  rs << s << ";\n";
}

void WireSet::BuildReadArg(const SignalDescription &arg_desc) {
  const Table &tab = res_.GetTable();
  ostream &rs = tab.ResourceSectionStream();
  string rwire = ResourceWireName(arg_desc);
  for (AccessorInfo *ac : accessors_) {
    AccessorSignal *rsig = ac->FindSignal(arg_desc);
    if (rsig == nullptr) {
      continue;
    }
    rs << "  assign " << AccessorEdgeWireName(*rsig)
       << " = " << rwire << ";\n";
  }
}

void WireSet::BuildNotifyAccessor(const SignalDescription &desc) {
  BuildReadArg(desc);
}

void WireSet::BuildNotifyParent(const SignalDescription &desc) {
  vector<string> wires;
  for (AccessorInfo *ac : accessors_) {
    AccessorSignal *sig = ac->FindSignal(desc);
    if (sig == nullptr) {
      continue;
    }
    wires.push_back(AccessorEdgeWireName(*sig));
  }
  const Table &tab = res_.GetTable();
  ostream &rs = tab.ResourceSectionStream();
  rs << "  assign " << ResourceWireName(desc) << " = ";
  if (wires.size() == 0) {
    rs << "0";
  } else {
    rs << Util::Join(wires, " | ");
  }
  rs << ";\n";
}

void WireSet::BuildAccessorWire(const SignalDescription &desc) {
  vector<AccessorSignal *> accessors_sigs;
  for (AccessorInfo *accessor : accessors_) {
    auto &signals = accessor->GetSignals();
    for (auto *sig : signals) {
      if (sig->sig_desc_ == &desc) {
	accessors_sigs.push_back(sig);
      }
    }
  }
  InterModuleWire wire(res_);
  bool from_parent = true;
  auto type = desc.type_;
  if (type == AccessorSignalType::ACCESSOR_REQ ||
      type == AccessorSignalType::ACCESSOR_WRITE_ARG ||
      type == AccessorSignalType::ACCESSOR_NOTIFY_PARENT) {
    from_parent = false;
  }
  for (auto *ac : accessors_sigs) {
    string name = AccessorWireName(*ac);
    wire.AddWire(*ac->accessor_res_, name, ac->sig_desc_->width_,
		 from_parent);
  }
}

void WireSet::BuildResourceWire() {
  Module *mod = res_.GetTable().GetModule();
  auto *tmpl = mod->GetModuleTemplate();
  ostream &os = tmpl->GetStream(kInsnWireDeclSection);
  os << "  // Resource wires - " << resource_name_ << "\n";
  for (auto it : signal_desc_) {
    auto *sig_desc = it.second;
    os << "  wire " << Table::WidthSpec(sig_desc->width_)
       << ResourceWireName(*sig_desc) << ";\n";
  }
}

void WireSet::BuildDistanceRegs() {
  for (AccessorInfo *ac : accessors_) {
    DistanceRegs d(ac, this);
    d.Build();
  }
}

string WireSet::ResourceWireName(const SignalDescription &desc) {
  return Names::ResourceWire(resource_name_, desc.name_.c_str());
}

string WireSet::AccessorWireName(const AccessorSignal &sig) {
  return Names::AccessorWire(resource_name_, sig.accessor_res_,
			     sig.sig_desc_->name_.c_str());
}

string WireSet::AccessorEdgeWireName(const AccessorSignal &sig) {
  return Names::AccessorEdgeWire(resource_name_, sig.accessor_res_,
				 sig.sig_desc_->name_.c_str());
}

string WireSet::AccessorWireNameWithReg(const AccessorSignal &sig) {
  return Names::AccessorSignalBase(resource_name_, sig.accessor_res_,
				   sig.sig_desc_->name_.c_str()) + "_reg";
}

void WireSet::BuildArbitration(const SignalDescription &req_desc,
			       const SignalDescription &ack_desc) {
  const Table &tab = res_.GetTable();
  ostream &rs = tab.ResourceSectionStream();
  vector<AccessorInfo*> handshake_accessors;
  for (auto *ac : accessors_) {
    AccessorSignal *rsig = ac->FindSignal(req_desc);
    if (rsig == nullptr) {
      continue;
    }
    AccessorSignal *asig = ac->FindSignal(ack_desc);
    CHECK(asig != nullptr);
    handshake_accessors.push_back(ac);
  }
  rs << "  // Arbitration and handshake - " << resource_name_ << "\n";
  // Registered req.
  BuildRegisteredReq(req_desc, handshake_accessors);
  // Req.
  vector<string> req_sigs;
  for (auto *ac : handshake_accessors) {
    AccessorSignal *rsig = ac->FindSignal(req_desc);
    req_sigs.push_back(AccessorEdgeWireName(*rsig));
  }
  rs << "  assign " << ResourceWireName(req_desc) << " = "
     << Util::Join(req_sigs, " | ") << ";\n";
  // Accessor Acks.
  BuildAccessorAck(req_desc, ack_desc, handshake_accessors);
}

void WireSet::BuildAccessorAck(const SignalDescription &req_desc,
			       const SignalDescription &ack_desc,
			       vector<AccessorInfo *> &handshake_accessors) {
  const Table &tab = res_.GetTable();
  ostream &rs = tab.ResourceSectionStream();
  string resource_ack = ResourceWireName(ack_desc);
  vector<string> high_reqs;
  for (auto *ac : handshake_accessors) {
    AccessorSignal *asig = ac->FindSignal(ack_desc);
    AccessorSignal *rsig = ac->FindSignal(req_desc);
    string req = AccessorEdgeWireName(*rsig);
    // ack = resource_ack & req & !(req from higher accessors).
    rs << "  assign " << AccessorEdgeWireName(*asig) << " = "
       << resource_ack << " & " << req;
    if (high_reqs.size() > 0) {
      rs << " & !(" <<  Util::Join(high_reqs, " | ") << ")";
    }
    rs << ";\n";
    high_reqs.push_back(AccessorWireNameWithReg(*rsig));
  }
}

void WireSet::BuildRegisteredReq(const SignalDescription &req_desc,
				 vector<AccessorInfo *> &handshake_accessors) {
  const Table &tab = res_.GetTable();
  ostream &rs = tab.ResourceSectionStream();
  ostream &vs = tab.ResourceValueSectionStream();
  string initial, body;
  for (auto *ac : handshake_accessors) {
    AccessorSignal *rsig = ac->FindSignal(req_desc);
    string reg = AccessorWireNameWithReg(*rsig);
    string wire = AccessorEdgeWireName(*rsig);
    rs << "  reg " << reg << ";\n";
    initial += "      " + reg + " <= 0;\n";
    body += "      " + reg + " <= " + wire + ";\n";
  }
  tab.WriteAlwaysBlockHead(vs);
  vs << initial;
  tab.WriteAlwaysBlockMiddle(vs);
  vs << body;
  tab.WriteAlwaysBlockTail(vs);
}

SignalDescription *WireSet::GetSignalDescription(const string &name,
						 AccessorSignalType type,
						 int width) {
  auto it = signal_desc_.find(name);
  if (it == signal_desc_.end()) {
    SignalDescription *sd = new SignalDescription();
    sd->name_ = name;
    sd->type_ = type;
    sd->width_ = width;
    signal_desc_[name] = sd;
    return sd;
  }
  return it->second;
}

string WireSet::GetResourceName() const {
  return resource_name_;
}

Resource &WireSet::GetResource() const {
  return res_;
}

}  // namespace wire
}  // namespace verilog
}  // namespace writer
}  // namespace iroha
