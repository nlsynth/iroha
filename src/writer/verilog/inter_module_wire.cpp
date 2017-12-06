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
  string drive = "wire";
  if (drive_by_reg) {
    drive = "reg";
  }
  Module *mod = res_.GetTable().GetModule();
  bool same_module = false;
  if (accessor.GetTable()->GetModule() == mod->GetIModule()) {
    same_module = true;
  }
  string a = Table::WidthSpec(width) + name + ";\n";
  // Accessor.
  IModule *accessor_imodule = accessor.GetTable()->GetModule();
  Module *accessor_module = mod->GetByIModule(accessor_imodule);
  if (!HasWire(accessor_module, name)) {
    auto *tmpl_a = accessor_module->GetModuleTemplate();
    ostream &rs_a = res_.GetTable().ResourceSectionStream();
    if (from_parent) {
      if (!same_module) {
	rs_a << "  wire " << a;
	AddWire(accessor_module, name);
      }
    } else {
      rs_a << "  " << drive << " " << a;
      AddWire(accessor_module, name);
    }
  }
  // (parent) Resource.
  if (!HasWire(mod, name)) {
    auto *tmpl_p = mod->GetModuleTemplate();
    ostream &rs_p = res_.GetTable().ResourceSectionStream();
    if (from_parent) {
      rs_p << "  " << drive << " " << a;
      AddWire(mod, name);
    } else {
      if (!same_module) {
	rs_p << "  wire " << a;
	AddWire(mod, name);
      }
    }
  }
  // Path.
  const IModule *common_root = Connection::GetCommonRoot(mod->GetIModule(),
							 accessor_imodule);
  if (mod->GetIModule() != common_root && accessor_imodule != common_root) {
    Module *m = mod->GetByIModule(common_root);
    if (!HasWire(m, name)) {
      auto *tmpl = m->GetModuleTemplate();
      ostream &rs = res_.GetTable().ResourceSectionStream();
      rs << "  wire " << a;
      AddWire(m, name);
    }
  }
  // upward
  for (const IModule *imod = mod->GetIModule(); imod != common_root;
       imod = imod->GetParentModule()) {
    Module *m = mod->GetByIModule(imod);
    bool is_upward = false;
    if (from_parent) {
      is_upward = true;
    }
    AddPort(m, name, width, is_upward);
  }
  // downward
  for (const IModule *imod = accessor_imodule; imod != common_root;
       imod = imod->GetParentModule()) {
    Module *m = mod->GetByIModule(imod);
    bool is_upward = true;
    if (from_parent) {
      is_upward = false;
    }
    AddPort(m, name, width, is_upward);
  }
}

void InterModuleWire::AddPort(Module *mod, const string &name, int width,
			      bool upward) {
  // Use a different key from wires.
  string wire_key = "port:" + name;
  if (HasWire(mod, wire_key)) {
    return;
  } else {
    AddWire(mod, wire_key);
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

void InterModuleWire::AddSharedWires(const vector<IResource *> &accessors,
				     const string &name, int width,
				     bool from_parent, bool drive_by_reg) {
  for (auto *a : accessors) {
    AddWire(*a, name, width, from_parent, drive_by_reg);
  }
}

bool InterModuleWire::HasWire(Module *mod, const string &name) {
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

void InterModuleWire::AddWire(Module *mod, const string &name) {
  has_wire_[mod].insert(name);
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
