#include "writer/verilog/wire/wire_set.h"

#include "iroha/i_design.h"
#include "iroha/stl_util.h"
#include "iroha/logging.h"
#include "writer/module_template.h"
#include "writer/verilog/embed.h"
#include "writer/verilog/module.h"
#include "writer/verilog/ports.h"
#include "writer/verilog/port_set.h"
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
// accessor resource -> accessor wire -> distance registers
//  -> accesor edge wire
//  -> arbitration/handshake logic (Mux)
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
  rs << DEBUG_MESSAGE("  // WireSet end");
  // Hands over the ownership as well.
  tab.GetEmbeddedModules()->RequestWireMux(this);
  Module *mod = res_.GetTable().GetModule();
  auto *tmpl = mod->GetModuleTemplate();
  ostream &es = tmpl->GetStream(kEmbeddedInstanceSection);
  string name = GetMuxName();
  PortSet *pp = tab.GetPortSet();
  string clk = pp->GetClk();
  string rst = pp->GetReset();
  es << "  " << name << " inst_" << name <<
    "(." << clk << "(" << clk << "), ." << rst << "(" << rst << ")";
  for (AccessorInfo *ac : accessors_) {
    auto &asigs = ac->GetSignals();
    for (auto &asig : asigs) {
      string s = AccessorEdgeWireName(*asig);
      es << ", ." << s << "(" << s << ")";
    }
  }
  for (auto it : signal_desc_) {
    auto *sig_desc = it.second;
    string s = ResourceWireName(*sig_desc);
    es << ", ." << s << "(" << s << ")";
  }
  es << ");\n";
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

string WireSet::ResourceWireName(const SignalDescription &desc) const {
  return Names::ResourceWire(resource_name_, desc.name_.c_str());
}

string WireSet::AccessorWireName(const AccessorSignal &sig) {
  return Names::AccessorWire(resource_name_, sig.accessor_res_,
			     sig.sig_desc_->name_.c_str());
}

string WireSet::AccessorEdgeWireName(const AccessorSignal &sig) const {
  return Names::AccessorEdgeWire(resource_name_, sig.accessor_res_,
				 sig.sig_desc_->name_.c_str());
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
    sd->default0_ = false;
    signal_desc_[name] = sd;
    return sd;
  }
  return it->second;
}

const vector<AccessorInfo *> &WireSet::GetAccessors() const {
  return accessors_;
}

vector<SignalDescription *> WireSet::GetSignals() const {
  vector<SignalDescription *> sigs;
  for (auto sd : signal_desc_) {
    sigs.push_back(sd.second);
  }
  return sigs;
}

string WireSet::GetResourceName() const {
  return resource_name_;
}

Resource &WireSet::GetResource() const {
  return res_;
}

string WireSet::GetMuxName() const {
  return "mux_" + resource_name_;
}

}  // namespace wire
}  // namespace verilog
}  // namespace writer
}  // namespace iroha
