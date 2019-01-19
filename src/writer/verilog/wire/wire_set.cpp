#include "writer/verilog/wire/wire_set.h"

#include "iroha/i_design.h"
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
  asig.sig_desc_ = wire_set_->GetSignalDescription(name, type, width);
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

AccessorSignal *AccessorInfo::FindSignal(const SignalDescription &sig_desc) {
  for (auto &asig : accessor_signals_) {
    if (asig.sig_desc_ == &sig_desc) {
      return &asig;
    }
  }
  return nullptr;
}

string AccessorInfo::AccessorName(const IResource *res) {
  if (res == nullptr) {
    return "";
  }
  ITable *tab = res->GetTable();
  IModule *mod = tab->GetModule();
  return Util::Itoa(mod->GetId()) + "_" + Util::Itoa(tab->GetId()) + "_" + Util::Itoa(res->GetId());
}

WireSet::WireSet(Resource &res, const string &name) : res_(res), name_(name) {
}

WireSet::~WireSet() {
  STLDeleteValues(&accessors_);
  STLDeleteSecondElements(&signal_desc_);
}

AccessorInfo *WireSet::AddAccessor(IResource *accessor, const string &name) {
  AccessorInfo *info = new AccessorInfo(this, accessor, name);
  accessors_.push_back(info);
  return info;
}

void WireSet::Build() {
  SignalDescription *rsig_desc = nullptr;
  SignalDescription *asig_desc = nullptr;
  for (auto it : signal_desc_) {
    SignalDescription *sig_desc = it.second;
    if (sig_desc->type_ == AccessorSignalType::ACCESSOR_REQ) {
      rsig_desc = sig_desc;
    }
    if (sig_desc->type_ == AccessorSignalType::ACCESSOR_ACK) {
      asig_desc = sig_desc;
    }
  }
  for (auto it : signal_desc_) {
    SignalDescription *sig_desc = it.second;
    BuildAccessorWire(*sig_desc);
  }
  BuildResourceWire();
  BuildArbitration(*rsig_desc, *asig_desc);
  for (auto it : signal_desc_) {
    SignalDescription *sig_desc = it.second;
    if (sig_desc->type_ == AccessorSignalType::ACCESSOR_WRITE_ARG) {
      BuildWriteArg(*sig_desc, *rsig_desc);
    }
    if (sig_desc->type_ == AccessorSignalType::ACCESSOR_READ_ARG) {
      BuildReadArg(*sig_desc);
    }
  }
}

void WireSet::BuildWriteArg(const SignalDescription &sig_desc,
			    const SignalDescription &req_sig_desc) {
  const Table &tab = res_.GetTable();
  ostream &rs = tab.ResourceSectionStream();
  rs << "  assign " << ResourceWireName(sig_desc) << " = ";
  // wire names of write_arg, req
  vector<pair<string, string> > pins;
  for (AccessorInfo *ac : accessors_) {
    AccessorSignal *rsig = ac->FindSignal(req_sig_desc);
    AccessorSignal *warg = ac->FindSignal(sig_desc);
    if (rsig == nullptr || warg == nullptr) {
      continue;
    }
    pins.push_back(make_pair(AccessorWireName(*warg), AccessorWireName(*rsig)));
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

void WireSet::BuildReadArg(const SignalDescription &sig_desc) {
  const Table &tab = res_.GetTable();
  ostream &rs = tab.ResourceSectionStream();
  for (AccessorInfo *ac : accessors_) {
    AccessorSignal *rsig = ac->FindSignal(sig_desc);
    rs << "  assign " << AccessorWireName(*rsig) << " = " << ResourceWireName(sig_desc) << ";\n";
  }
}

void WireSet::BuildAccessorWire(const SignalDescription &sig_desc) {
  vector<AccessorSignal> accessors_sigs;
  for (AccessorInfo *accessor : accessors_) {
    auto &signals = accessor->GetSignals();
    for (auto &sig : signals) {
      if (sig.sig_desc_ == &sig_desc) {
	accessors_sigs.push_back(sig);
      }
    }
  }
  InterModuleWire wire(res_);
  bool from_parent = true;
  bool driven_by_reg = false;
  auto type = sig_desc.type_;
  if (type == AccessorSignalType::ACCESSOR_REQ ||
      type == AccessorSignalType::ACCESSOR_WRITE_ARG) {
    from_parent = false;
  }
  for (auto &ac : accessors_sigs) {
    string name = ac.accessor_info_->GetName() + "_" + ac.sig_desc_->name_ + "_wire";
    wire.AddWire(*ac.accessor_res_, name, ac.sig_desc_->width_,
		 from_parent, driven_by_reg);
  }
}

void WireSet::BuildResourceWire() {
  Module *mod = res_.GetTable().GetModule();
  auto *tmpl = mod->GetModuleTemplate();
  ostream &os = tmpl->GetStream(kInsnWireDeclSection);
  os << "  // Resource wires - " << name_ << "\n";
  for (auto it : signal_desc_) {
    auto *sig_desc = it.second;
    os << "  wire " << Table::WidthSpec(sig_desc->width_) << ResourceWireName(*sig_desc) << ";\n";
  }
}

string WireSet::ResourceWireName(const SignalDescription &sig_desc) {
  return name_ + "_" + sig_desc.name_ + "_wire";
}

string WireSet::AccessorWireName(const AccessorSignal &sig) {
  return sig.accessor_info_->GetName() + "_" + sig.sig_desc_->name_ + "_wire";
}

string WireSet::AccessorWireNameWithReg(const AccessorSignal &sig) {
  return sig.accessor_info_->GetName() + "_" + sig.sig_desc_->name_ + "_reg";
}

void WireSet::BuildArbitration(const SignalDescription &rsig_desc, const SignalDescription &asig_desc) {
  const Table &tab = res_.GetTable();
  ostream &rs = tab.ResourceSectionStream();
  vector<AccessorInfo*> handshake_accessors;
  for (auto *ac : accessors_) {
    AccessorSignal *rsig = ac->FindSignal(rsig_desc);
    if (rsig == nullptr) {
      continue;
    }
    AccessorSignal *asig = ac->FindSignal(asig_desc);
    CHECK(asig != nullptr);
    handshake_accessors.push_back(ac);
  }
  rs << "  // Arbitration and handshake - " << name_ << "\n";
  // Registered req.
  BuildRegisteredReq(rsig_desc, handshake_accessors);
  // Req.
  vector<string> req_sigs;
  for (auto *ac : handshake_accessors) {
    AccessorSignal *rsig = ac->FindSignal(rsig_desc);
    req_sigs.push_back(AccessorWireName(*rsig));
  }
  rs << "  assign " << ResourceWireName(rsig_desc) << " = " << Util::Join(req_sigs, " | ") << ";\n";
  // Accessor Acks.
  BuildAccessorAck(rsig_desc, asig_desc, handshake_accessors);
}

void WireSet::BuildAccessorAck(const SignalDescription &rsig_desc, const SignalDescription &asig_desc,
			       vector<AccessorInfo *> &handshake_accessors) {
  const Table &tab = res_.GetTable();
  ostream &rs = tab.ResourceSectionStream();
  string resource_ack = ResourceWireName(asig_desc);
  vector<string> high_reqs;
  for (auto *ac : handshake_accessors) {
    AccessorSignal *asig = ac->FindSignal(asig_desc);
    AccessorSignal *rsig = ac->FindSignal(rsig_desc);
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

void WireSet::BuildRegisteredReq(const SignalDescription &rsig_desc,
				 vector<AccessorInfo *> &handshake_accessors) {
  const Table &tab = res_.GetTable();
  ostream &rs = tab.ResourceSectionStream();
  ostream &vs = tab.ResourceValueSectionStream();
  string initial, body;
  for (auto *ac : handshake_accessors) {
    AccessorSignal *rsig = ac->FindSignal(rsig_desc);
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

SignalDescription *WireSet::GetSignalDescription(const string &name, AccessorSignalType type, int width) {
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

}  // namespace wire
}  // namespace verilog
}  // namespace writer
}  // namespace iroha
