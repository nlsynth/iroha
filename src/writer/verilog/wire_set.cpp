#include "writer/verilog/wire_set.h"

#include "iroha/stl_util.h"
#include "iroha/logging.h"
#include "writer/module_template.h"
#include "writer/verilog/inter_module_wire.h"
#include "writer/verilog/module.h"
#include "writer/verilog/resource.h"
#include "writer/verilog/table.h"

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

AccessorInfo::AccessorInfo(IResource *accessor, const string &name)
  : accessor_(accessor), name_(name) {
}

void AccessorInfo::AddSignal(const string &name, AccessorSignalType type,
			     int width) {
  AccessorSignal sig;
  sig.name_ = name;
  sig.type_ = type;
  sig.width_ = width;
  sig.accessor_res_ = accessor_;
  sig.info_ = this;
  signals_.push_back(sig);
}

const vector<AccessorSignal> &AccessorInfo::GetSignals() {
  return signals_;
}

const string &AccessorInfo::GetName() {
  return name_;
}

AccessorSignal *AccessorInfo::FindSignal(const string &name) {
  for (auto &sig : signals_) {
    if (sig.name_ == name) {
      return &sig;
    }
  }
  return nullptr;
}

WireSet::WireSet(Resource &res, const string &name) : res_(res), name_(name) {
}

WireSet::~WireSet() {
  STLDeleteValues(&accessors_);
}

AccessorInfo *WireSet::AddAccessor(IResource *accessor, const string &name) {
  AccessorInfo *info = new AccessorInfo(accessor, name);
  accessors_.push_back(info);
  return info;
}

void WireSet::Build() {
  set<string> sig_names;
  // Pick one for each of the name_ (e.g. ack, req, wdata).
  vector<AccessorSignal> uniq_signals;
  for (AccessorInfo *accessor : accessors_) {
    auto &signals = accessor->GetSignals();
    for (auto &sig : signals) {
      if (sig_names.find(sig.name_) == sig_names.end()) {
	sig_names.insert(sig.name_);
	uniq_signals.push_back(sig);
      }
    }
  }
  AccessorSignal *req = nullptr;
  AccessorSignal *ack = nullptr;
  for (auto &sig : uniq_signals) {
    if (sig.type_ == AccessorSignalType::ACCESSOR_REQ) {
      req = &sig;
    }
    if (sig.type_ == AccessorSignalType::ACCESSOR_ACK) {
      ack = &sig;
    }
  }
  for (auto &sig : uniq_signals) {
    BuildAccessorWire(sig);
  }
  BuildResourceWire(uniq_signals);
  CHECK(req != nullptr);
  CHECK(ack != nullptr);
  BuildArbitration(*req, *ack);
}

void WireSet::BuildAccessorWire(const AccessorSignal &primary_sig) {
  vector<AccessorSignal> accessors_sigs;
  for (AccessorInfo *accessor : accessors_) {
    auto &signals = accessor->GetSignals();
    for (auto &sig : signals) {
      if (sig.name_ == primary_sig.name_) {
	accessors_sigs.push_back(sig);
      }
    }
  }
  InterModuleWire wire(res_);
  bool from_parent = true;
  bool driven_by_reg = false;
  auto type = primary_sig.type_;
  if (type == AccessorSignalType::ACCESSOR_REQ ||
      type == AccessorSignalType::ACCESSOR_WRITE_ARG) {
    from_parent = false;
  }
  for (auto &ac : accessors_sigs) {
    string name = ac.info_->GetName() + "_" + ac.name_ + "_wire";
    wire.AddWire(*ac.accessor_res_, name, ac.width_,
		 from_parent, driven_by_reg);
  }
}

void WireSet::BuildResourceWire(const vector<AccessorSignal> &uniq_signals) {
  Module *mod = res_.GetTable().GetModule();
  auto *tmpl = mod->GetModuleTemplate();
  ostream &os = tmpl->GetStream(kInsnWireDeclSection);
  os << "  // Resource wires - " << name_ << "\n";
  for (auto &sig : uniq_signals) {
    os << "  wire " << Table::WidthSpec(sig.width_) << ResourceWireName(sig) << ";\n";
  }
}

string WireSet::ResourceWireName(const AccessorSignal &sig) {
  return name_ + "_" + sig.name_;
}

string WireSet::AccessorWireName(const AccessorSignal &sig) {
  return sig.info_->GetName() + "_" + sig.name_ + "_wire";
}

void WireSet::BuildArbitration(const AccessorSignal &req, const AccessorSignal &ack) {
  ostream &rs = res_.GetTable().ResourceSectionStream();
  vector<string> req_sigs;
  for (auto *ac : accessors_) {
    AccessorSignal *rsig = ac->FindSignal(req.name_);
    if (rsig == nullptr) {
      continue;
    }
    req_sigs.push_back(AccessorWireName(*rsig));
  }
  rs << "  // Arbitration and handshake - " << name_ << "\n";
  rs << "  assign " << ResourceWireName(req) << " = " << Util::Join(req_sigs, " | ") << ";\n";
  string resource_ack = ResourceWireName(ack);
  // WIP: This is driven by the resource.
  rs << "  assign " << resource_ack << " = 0;\n";
  vector<string> high_reqs;
  for (auto *ac : accessors_) {
    AccessorSignal *asig = ac->FindSignal(ack.name_);
    if (asig == nullptr) {
      continue;
    }
    AccessorSignal *rsig = ac->FindSignal(req.name_);
    CHECK(rsig != nullptr);
    string req = AccessorWireName(*rsig);
    // TODO: WIP: Fix to add a reg to each req signal.
    // ack = resource_ack & req & !(req from higher accessors).
    rs << "  assign " << AccessorWireName(*asig) << " = "
       << resource_ack << " & " << req;
    if (high_reqs.size() > 0) {
      rs << " & !(" <<  Util::Join(high_reqs, " | ") << ")";
    }
    rs << ";\n";
    high_reqs.push_back(AccessorWireName(*rsig));
  }
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
