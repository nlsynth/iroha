#include "writer/verilog/shared_reg.h"

#include "iroha/i_design.h"
#include "iroha/logging.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"
#include "writer/connection.h"
#include "writer/module_template.h"
#include "writer/verilog/insn_writer.h"
#include "writer/verilog/module.h"
#include "writer/verilog/ports.h"
#include "writer/verilog/state.h"
#include "writer/verilog/table.h"

namespace iroha {
namespace writer {
namespace verilog {

SharedReg::SharedReg(const IResource &res, const Table &table)
  : Resource(res, table), has_default_output_value_(false),
    default_output_value_(0) {
  auto *klass = res_.GetClass();
  if (resource::IsSharedReg(*klass)) {
    auto *params = res_.GetParams();
    string unused;
    params->GetExtOutputPort(&unused, &width_);
    has_default_output_value_ =
      params->GetDefaultValue(&default_output_value_);
  } else {
    width_ = 0;
  }
}

void SharedReg::BuildResource() {
  auto *klass = res_.GetClass();
  if (resource::IsSharedReg(*klass)) {
    ostream &rs = tmpl_->GetStream(kRegisterSection);
    rs << "  // shared-reg\n";
    rs << "  reg ";
    if (width_ > 0) {
      rs << "[" << width_ - 1 << ":0]";
    }
    rs << " " << RegName(res_) << ";\n";
    if (has_default_output_value_) {
      ostream &os = tab_.StateOutputSectionStream();
      os << "      " << RegName(res_) << " <= "
	 << SelectValueByState(default_output_value_) << ";\n";
    }
    // Reset value
    ostream &is = tab_.InitialValueSectionStream();
    is << "      " << RegName(res_) << " <= ";
    if (has_default_output_value_) {
      is << default_output_value_;
    } else {
      is << 0;
    }
    is << ";\n";
  }
}

void SharedReg::BuildInsn(IInsn *insn, State *st) {
  auto *klass = res_.GetClass();
  if (resource::IsSharedReg(*klass)) {
    if (!has_default_output_value_ &&
	insn->inputs_.size() == 1) {
      ostream &os = st->StateBodySectionStream();
      os << "          " << RegName(res_) << " <= "
	 << InsnWriter::RegisterName(*insn->inputs_[0])
	 << ";\n";
    }
    if (insn->outputs_.size() == 1) {
      // Read from self.
      ostream &ws = tmpl_->GetStream(kInsnWireValueSection);
      ws << "  assign "
	 << InsnWriter::InsnOutputWireName(*insn, 0)
	 << " = "
	 << RegName(res_) << ";\n";
    }
  }
  if (resource::IsSharedRegReader(*klass)) {
    // Read from another table.
    IResource *source = res_.GetSharedReg();
    ostream &ws = tmpl_->GetStream(kInsnWireValueSection);
    ws << "  assign "
       << InsnWriter::InsnOutputWireName(*insn, 0)
       << " = "
       << RegName(*source) << ";\n";
  }
}

string SharedReg::RegName(const IResource &res) {
  auto *params = res.GetParams();
  int unused_width;
  string port_name;
  params->GetExtOutputPort(&port_name, &unused_width);
  ITable *tab = res.GetTable();
  IModule *mod = tab->GetModule();
  return "port_output_" +
    Util::Itoa(mod->GetId()) + "_" +
    Util::Itoa(tab->GetId()) + "_" + Util::Itoa(res.GetId()) +
    "_" + port_name;
}

void SharedReg::BuildPorts(const PortConnectionInfo &pi, Ports *ports) {
  for (IResource *res : pi.has_upward_port) {
    int width = res->GetParams()->GetWidth();
    ports->AddPort(RegName(*res), Port::OUTPUT_WIRE, width);
  }
  for (IResource *res : pi.has_downward_port) {
    int width = res->GetParams()->GetWidth();
    ports->AddPort(RegName(*res), Port::INPUT, width);
  }
}

void SharedReg::BuildChildWire(const PortConnectionInfo &pi, ostream &os) {
  for (IResource *res : pi.has_upward_port) {
    AddChildWire(res, os);
  }
  for (IResource *res : pi.has_downward_port) {
    AddChildWire(res, os);
  }
}

void SharedReg::AddChildWire(IResource *res, ostream &os) {
  string name = RegName(*res);
  os << ", ." << name << "(" << name << ")";
}

void SharedReg::BuildRootWire(const PortConnectionInfo &pi, Module *module) {
  ModuleTemplate *tmpl = module->GetModuleTemplate();
  ostream &ws = tmpl->GetStream(kInsnWireDeclSection);
  for (IResource *res : pi.has_wire) {
    ws << "  wire ";
    int width = res->GetParams()->GetWidth();
    if (width > 0) {
      ws << "[" << width - 1 << ":0] ";
    }
    ws << RegName(*res) << ";\n";
  }
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
