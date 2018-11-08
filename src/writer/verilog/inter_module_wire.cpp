#include "writer/verilog/inter_module_wire.h"

#include "iroha/i_design.h"
#include "writer/connection.h"
#include "writer/module_template.h"
#include "writer/verilog/module.h"
#include "writer/verilog/ports.h"
#include "writer/verilog/resource.h"
#include "writer/verilog/table.h"

namespace iroha {
namespace writer {
namespace verilog {

InterModuleWire::InterModuleWire(Resource &res) : res_(res) {
}

void InterModuleWire::AddWire(IResource &accessor, const string &name,
			      int width, bool from_parent, bool drive_by_reg) {
  AddWireConnection(accessor, name, width, from_parent, drive_by_reg);
  AddWireEdge(accessor, name, width, from_parent, drive_by_reg);
}

void InterModuleWire::AddWireEdge(const IResource &accessor, const string &name,
				  int width, bool from_parent,
				  bool drive_by_reg) {
  Module *parent_mod = res_.GetTable().GetModule();
  const IModule *parent_imodule = parent_mod->GetIModule();
  IModule *accessor_imodule = accessor.GetTable()->GetModule();
  const Module *accessor_module = parent_mod->GetByIModule(accessor_imodule);
  const IModule *common_root =
    Connection::GetCommonRoot(parent_imodule,
			      accessor_imodule);
  bool same_module = false;
  if (accessor_imodule == parent_imodule) {
    same_module = true;
  }
  string a = Table::WidthSpec(width) + name + ";\n";
  if (!HasWire(accessor_module, name) && !HasPort(accessor_module, name)) {
    if (from_parent && common_root == accessor_imodule && !same_module) {
      auto *tmpl_a = accessor_module->GetModuleTemplate();
      ostream &rs_a = tmpl_a->GetStream(kInsnWireDeclSection);
      rs_a << "  wire " << a;
      AddWireName(accessor_module, name);
    }
  }
  if (!HasWire(parent_mod, name) && !HasPort(parent_mod, name)) {
    if (!from_parent && common_root == parent_imodule &&
	!same_module) {
      auto *tmpl_p = parent_mod->GetModuleTemplate();
      ostream &rs_p = res_.GetTable().ResourceSectionStream();
      rs_p << "  wire " << a;
      AddWireName(parent_mod, name);
    }
  }
}

void InterModuleWire::AddWireConnection(const IResource &accessor, const string &name,
					int width, bool from_parent,
					bool drive_by_reg) {
  string drive = "wire";
  if (drive_by_reg) {
    drive = "reg";
  }
  Module *parent_mod = res_.GetTable().GetModule();
  const IModule *parent_imodule = parent_mod->GetIModule();
  IModule *accessor_imodule = accessor.GetTable()->GetModule();
  const Module *accessor_module = parent_mod->GetByIModule(accessor_imodule);
  string a = Table::WidthSpec(width) + name + ";\n";
  // Accessor.
  const IModule *common_root =
    Connection::GetCommonRoot(parent_imodule,
			      accessor_imodule);
  if (!HasWire(accessor_module, name)) {
    auto *tmpl_a = accessor_module->GetModuleTemplate();
    ostream &rs_a = tmpl_a->GetStream(kInsnWireDeclSection);
    if (!from_parent) {
      rs_a << "  " << drive << " " << a;
      AddWireName(accessor_module, name);
    }
  }
  // (parent) Resource.
  if (!HasWire(parent_mod, name)) {
    auto *tmpl_p = parent_mod->GetModuleTemplate();
    ostream &rs_p = res_.GetTable().ResourceSectionStream();
    if (from_parent) {
      rs_p << "  " << drive << " " << a;
      AddWireName(parent_mod, name);
    }
  }
  // Path.
  if (parent_mod->GetIModule() != common_root &&
      accessor_imodule != common_root) {
    Module *m = parent_mod->GetByIModule(common_root);
    if (!HasWire(m, name)) {
      auto *tmpl = m->GetModuleTemplate();
      ostream &rs = tmpl->GetStream(kInsnWireDeclSection);
      rs << "  wire " << a;
      AddWireName(m, name);
    }
  }
  // upward
  for (const IModule *imod = parent_mod->GetIModule(); imod != common_root;
       imod = imod->GetParentModule()) {
    Module *m = parent_mod->GetByIModule(imod);
    bool is_upward = false;
    if (from_parent) {
      is_upward = true;
    }
    AddPort(m, name, width, is_upward);
  }
  // downward
  for (const IModule *imod = accessor_imodule; imod != common_root;
       imod = imod->GetParentModule()) {
    Module *m = parent_mod->GetByIModule(imod);
    bool is_upward = true;
    if (from_parent) {
      is_upward = false;
    }
    AddPort(m, name, width, is_upward);
  }
}

void InterModuleWire::AddPort(Module *mod, const string &name, int width,
			      bool upward) {
  if (HasPort(mod, name)) {
    return;
  } else {
    // Use a different key from wires.
    string wire_key = "port:" + name;
    AddWireName(mod, wire_key);
  }

  Ports *ports = mod->GetPorts();
  if (upward) {
    ports->AddPort(name, Port::OUTPUT_WIRE, width);
  } else {
    ports->AddPort(name, Port::INPUT, width);
  }
  Module *parent_mod = mod->GetParentModule();
  ostream &os = parent_mod->ChildModuleInstSectionStream(mod);
  os << ", ." << name << "(" << name << ")";
}

void InterModuleWire::AddSharedWires(const vector<const IResource *> &accessors,
				     const string &name, int width,
				     bool from_parent, bool drive_by_reg) {
  for (auto *a : accessors) {
    AddWireConnection(*a, name, width, from_parent, drive_by_reg);
  }
  for (auto *a : accessors) {
    AddWireEdge(*a, name, width, from_parent, drive_by_reg);
  }
}

bool InterModuleWire::HasPort(const Module *mod, const string &name) {
  return HasWire(mod, "port:" + name);
}

bool InterModuleWire::HasWire(const Module *mod, const string &name) {
  auto it = has_wire_.find(mod);
  if (it == has_wire_.end()) {
    return false;
  }
  auto s = it->second;
  if (s.find(name) == s.end()) {
    return false;
  }
  return true;
}

void InterModuleWire::AddWireName(const Module *mod, const string &name) {
  has_wire_[mod].insert(name);
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
