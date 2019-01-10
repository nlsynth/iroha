#include "writer/verilog/wire/wire_set.h"

#include "iroha/stl_util.h"
#include "iroha/logging.h"
#include "writer/module_template.h"
#include "writer/verilog/module.h"
#include "writer/verilog/resource.h"
#include "writer/verilog/table.h"
#include "writer/verilog/wire/inter_module_wire.h"

// WIP: Unified interconnect wiring system for
// shared_memory, shared_reg, fifo, task (and study)
// This handles:
//  * Handshake between accessors and a parent.
//  * Arbitration between multiple accessors.
//  * Distance
//  * Hierarchy
//
// NOTE:
// * shared_memory
//   reader/writer/owner
// * shared_reg
//   reader/writer/owner
//   normal access, put/get, notify
// * fifo
//   reader - writer
// * task
//   caller
// * study
//   -
//
// Usage:
// WireSet ws(parent, "name");
// AccessorInfo *ac = ws.AddAccessor(writer, "name_parent_accessor");
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
//  -> arbitration/handshake logic
//  -> resource wire -> parent resource


namespace iroha {
namespace writer {
namespace verilog {
namespace wire {

AccessorInfo::AccessorInfo(WireSet *wire_set, IResource *accessor, const string &name)
  : wire_set_(wire_set), accessor_(accessor), name_(name) {
}

void AccessorInfo::AddSignal(const string &name, AccessorSignalType type,
			     int width) {
  AccessorSignal asig;
  asig.sig_info_ = wire_set_->GetSignalInfo(name, type, width);
  asig.accessor_res_ = accessor_;
  asig.accessor_info_ = this;
  accessor_signals_.push_back(asig);
}

const vector<AccessorSignal> &AccessorInfo::GetSignals() {
  return accessor_signals_;
}

const string &AccessorInfo::GetName() {
  return name_;
}

AccessorSignal *AccessorInfo::FindSignal(const SignalInfo &sig_info) {
  for (auto &asig : accessor_signals_) {
    if (asig.sig_info_ == &sig_info) {
      return &asig;
    }
  }
  return nullptr;
}

WireSet::WireSet(Resource &res, const string &name) : res_(res), name_(name) {
}

WireSet::~WireSet() {
  STLDeleteValues(&accessors_);
  STLDeleteSecondElements(&signal_info_);
}

AccessorInfo *WireSet::AddAccessor(IResource *accessor, const string &name) {
  AccessorInfo *info = new AccessorInfo(this, accessor, name);
  accessors_.push_back(info);
  return info;
}

void WireSet::Build() {
  SignalInfo *rsig_info = nullptr;
  SignalInfo *asig_info = nullptr;
  for (auto it : signal_info_) {
    SignalInfo *sig_info = it.second;
    if (sig_info->type_ == AccessorSignalType::ACCESSOR_REQ) {
      rsig_info = sig_info;
    }
    if (sig_info->type_ == AccessorSignalType::ACCESSOR_ACK) {
      asig_info = sig_info;
    }
  }
  for (auto it : signal_info_) {
    SignalInfo *sig_info = it.second;
    BuildAccessorWire(*sig_info);
  }
  BuildResourceWire();
  BuildArbitration(*rsig_info, *asig_info);
  for (auto it : signal_info_) {
    SignalInfo *sig_info = it.second;
    if (sig_info->type_ == AccessorSignalType::ACCESSOR_WRITE_ARG) {
      BuildWriteArg(*sig_info, *rsig_info);
    }
  }
}

void WireSet::BuildWriteArg(const SignalInfo &sig_info, const SignalInfo &rsig_info) {
  const Table &tab = res_.GetTable();
  ostream &rs = tab.ResourceSectionStream();
  rs << "  assign " << ResourceWireName(sig_info) << " = ";
  vector<pair<string, string> > pins;
  for (AccessorInfo *ac : accessors_) {
    AccessorSignal *rsig = ac->FindSignal(rsig_info);
    AccessorSignal *warg = ac->FindSignal(sig_info);
    if (rsig == nullptr || warg == nullptr) {
      continue;
    }
    pins.push_back(make_pair(AccessorWireName(*warg), AccessorWireName(*rsig)));
  }
  string s;
  for (int i = pins.size() - 1; i > 0; --i) {
    if (s.empty()) {
      s = pins[i].first;
    } else {
      s = pins[i].second + " ? " + pins[i].first + " : (" + s + ")";
    }
  }
  rs << s << ";\n";
}

void WireSet::BuildAccessorWire(const SignalInfo &sig_info) {
  vector<AccessorSignal> accessors_sigs;
  for (AccessorInfo *accessor : accessors_) {
    auto &signals = accessor->GetSignals();
    for (auto &sig : signals) {
      if (sig.sig_info_ == &sig_info) {
	accessors_sigs.push_back(sig);
      }
    }
  }
  InterModuleWire wire(res_);
  bool from_parent = true;
  bool driven_by_reg = false;
  auto type = sig_info.type_;
  if (type == AccessorSignalType::ACCESSOR_REQ ||
      type == AccessorSignalType::ACCESSOR_WRITE_ARG) {
    from_parent = false;
  }
  for (auto &ac : accessors_sigs) {
    string name = ac.accessor_info_->GetName() + "_" + ac.sig_info_->name_ + "_wire";
    wire.AddWire(*ac.accessor_res_, name, ac.sig_info_->width_,
		 from_parent, driven_by_reg);
  }
}

void WireSet::BuildResourceWire() {
  Module *mod = res_.GetTable().GetModule();
  auto *tmpl = mod->GetModuleTemplate();
  ostream &os = tmpl->GetStream(kInsnWireDeclSection);
  os << "  // Resource wires - " << name_ << "\n";
  for (auto it : signal_info_) {
    auto *sig_info = it.second;
    os << "  wire " << Table::WidthSpec(sig_info->width_) << ResourceWireName(*sig_info) << ";\n";
  }
}

string WireSet::ResourceWireName(const SignalInfo &sig_info) {
  return name_ + "_" + sig_info.name_;
}

string WireSet::AccessorWireName(const AccessorSignal &sig) {
  return sig.accessor_info_->GetName() + "_" + sig.sig_info_->name_ + "_wire";
}

string WireSet::AccessorWireNameWithReg(const AccessorSignal &sig) {
  return sig.accessor_info_->GetName() + "_" + sig.sig_info_->name_ + "_reg";
}

void WireSet::BuildArbitration(const SignalInfo &rsig_info, const SignalInfo &asig_info) {
  const Table &tab = res_.GetTable();
  ostream &rs = tab.ResourceSectionStream();
  vector<AccessorInfo*> handshake_accessors;
  for (auto *ac : accessors_) {
    AccessorSignal *rsig = ac->FindSignal(rsig_info);
    if (rsig == nullptr) {
      continue;
    }
    AccessorSignal *asig = ac->FindSignal(asig_info);
    CHECK(asig != nullptr);
    handshake_accessors.push_back(ac);
  }
  rs << "  // Arbitration and handshake - " << name_ << "\n";
  // Registered req.
  BuildRegisteredReq(rsig_info, handshake_accessors);
  // Req.
  vector<string> req_sigs;
  for (auto *ac : handshake_accessors) {
    AccessorSignal *rsig = ac->FindSignal(rsig_info);
    req_sigs.push_back(AccessorWireName(*rsig));
  }
  rs << "  assign " << ResourceWireName(rsig_info) << " = " << Util::Join(req_sigs, " | ") << ";\n";
  // WIP: This is driven by the resource.
  string resource_ack = ResourceWireName(asig_info);
  rs << "  assign " << resource_ack << " = 0;\n";
  // Accessor Acks.
  BuildAccessorAck(rsig_info, asig_info, handshake_accessors);
}

void WireSet::BuildAccessorAck(const SignalInfo &rsig_info, const SignalInfo &asig_info,
			       vector<AccessorInfo *> &handshake_accessors) {
  const Table &tab = res_.GetTable();
  ostream &rs = tab.ResourceSectionStream();
  string resource_ack = ResourceWireName(asig_info);
  vector<string> high_reqs;
  for (auto *ac : handshake_accessors) {
    AccessorSignal *asig = ac->FindSignal(asig_info);
    AccessorSignal *rsig = ac->FindSignal(rsig_info);
    string req = AccessorWireName(*rsig);
    // ack = resource_ack & req & !(req from higher accessors).
    rs << "  assign " << AccessorWireName(*asig) << " = "
       << resource_ack << " & " << req;
    if (high_reqs.size() > 0) {
      rs << " & !(" <<  Util::Join(high_reqs, " | ") << ")";
    }
    rs << ";\n";
    high_reqs.push_back(AccessorWireNameWithReg(*rsig));
  }
}

void WireSet::BuildRegisteredReq(const SignalInfo &rsig_info,
				 vector<AccessorInfo *> &handshake_accessors) {
  const Table &tab = res_.GetTable();
  ostream &rs = tab.ResourceSectionStream();
  ostream &vs = tab.ResourceValueSectionStream();
  string initial, body;
  for (auto *ac : handshake_accessors) {
    AccessorSignal *rsig = ac->FindSignal(rsig_info);
    string reg = AccessorWireNameWithReg(*rsig);
    string wire = AccessorWireName(*rsig);
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

SignalInfo *WireSet::GetSignalInfo(const string &name, AccessorSignalType type, int width) {
  auto it = signal_info_.find(name);
  if (it == signal_info_.end()) {
    SignalInfo *si = new SignalInfo();
    si->name_ = name;
    si->type_ = type;
    si->width_ = width;
    signal_info_[name] = si;
    return si;
  }
  return it->second;
}

}  // namespace wire
}  // namespace verilog
}  // namespace writer
}  // namespace iroha
