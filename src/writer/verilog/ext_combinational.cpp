// WIP.
// TODO: Unify a bunch of code copied from ext_task_call.
// TODO: Support external module.
#include "writer/verilog/ext_combinational.h"

#include "iroha/i_design.h"
#include "iroha/resource_params.h"
#include "writer/module_template.h"
#include "writer/verilog/embed.h"
#include "writer/verilog/module.h"
#include "writer/verilog/ports.h"
#include "writer/verilog/table.h"

namespace iroha {
namespace writer {
namespace verilog {

ExtCombinational::ExtCombinational(const IResource &res, const Table &table)
  : Resource(res, table) {
}

void ExtCombinational::BuildResource() {
  string connection;
  for (int i = 0; i < res_.input_types_.size(); ++i) {
    AddPort(RetPin(&res_, i), RetPin(nullptr, i), false,
	    res_.input_types_[i].GetWidth(), &connection);
  }
  for (int i = 0; i < res_.output_types_.size(); ++i) {
    AddPort(ArgPin(&res_, i), ArgPin(nullptr, i), true,
	    res_.output_types_[i].GetWidth(), &connection);
  }
  BuildEmbeddedModule(connection);
}

void ExtCombinational::BuildInsn(IInsn *insn, State *st) {
}

void ExtCombinational::CollectNames(Names *names) {
}

void ExtCombinational::BuildEmbeddedModule(const string &connection) {
  auto *params = res_.GetParams();
  tab_.GetEmbeddedModules()->RequestModule(*params);

  auto *ports = tab_.GetPorts();
  ostream &is = tmpl_->GetStream(kEmbeddedInstanceSection);
  string name = params->GetEmbeddedModuleName();
  is << "  // " << name << "\n"
     << "  "  << name << " inst_" << tab_.GetITable()->GetId() << "_" << name
     << "(";
  is << "." << params->GetEmbeddedModuleClk() << "(" << ports->GetClk() << "), "
     << "." << params->GetEmbeddedModuleReset() << "(" << ports->GetReset() << ")";
  is << connection;
  is << ");\n";
}

void ExtCombinational::AddPort(const string &name, const string &wire_name,
			       bool is_output, int width,
			       string *connection) {
  ostream &rs = tab_.ResourceSectionStream();
  if (is_output) {
    rs << "  reg";
  } else {
    rs << "  wire";
  }
  rs << " " << Table::WidthSpec(width) << name << ";\n";
  *connection += ", ." + wire_name + "(" + name + ")";
}

string ExtCombinational::ArgPin(const IResource *res, int nth) {
  return PinName(res, "arg_" + Util::Itoa(nth));
}

string ExtCombinational::RetPin(const IResource *res, int nth) {
  return PinName(res, "ret_" + Util::Itoa(nth));
}

string ExtCombinational::PinName(const IResource *res, const string &name) {
  if (res != nullptr) {
    const string &prefix = res->GetParams()->GetExtTaskName();
    if (!prefix.empty()) {
      return res->GetParams()->GetExtTaskName() + "_" + name;
    }
  }
  return name;
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
