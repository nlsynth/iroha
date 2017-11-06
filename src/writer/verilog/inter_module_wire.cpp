#include "writer/verilog/inter_module_wire.h"

#include "iroha/i_design.h"
#include "writer/module_template.h"
#include "writer/verilog/module.h"
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
  // Accessor.
  IModule *accessor_imodule = accessor.GetTable()->GetModule();
  Module *accessor_module = mod->GetByIModule(accessor_imodule);
  auto *tmpl_a = accessor_module->GetModuleTemplate();
  ostream &rs_a = tmpl_a->GetStream(kResourceSection);
  string a = Table::WidthSpec(width) + name + ";\n";
  if (from_parent) {
    if (!same_module) {
      rs_a << "  wire " << a;
    }
  } else {
    rs_a << "  " << drive << " " << a;
  }
  // (parent) Resource.
  auto *tmpl_p = mod->GetModuleTemplate();
  ostream &rs_p = tmpl_p->GetStream(kResourceSection);
  if (from_parent) {
    rs_p << "  " << drive << " " << a;
  } else {
    if (!same_module) {
      rs_a << "  wire " << a;
    }
  }
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
