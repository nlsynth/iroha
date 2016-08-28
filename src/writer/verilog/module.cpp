#include "writer/verilog/module.h"

#include "design/design_util.h"
#include "iroha/i_design.h"
#include "iroha/resource_params.h"
#include "iroha/stl_util.h"
#include "writer/connection.h"
#include "writer/module_template.h"
#include "writer/verilog/channel.h"
#include "writer/verilog/ports.h"
#include "writer/verilog/shared_reg.h"
#include "writer/verilog/table.h"
#include "writer/verilog/sub_module_task.h"

namespace iroha {
namespace writer {
namespace verilog {

Module::Module(const IModule *i_mod, const Connection &conn,
	       EmbeddedModules *embed)
  : i_mod_(i_mod), conn_(conn), embed_(embed) {
  tmpl_.reset(new ModuleTemplate);
  ports_.reset(new Ports);
  reset_polarity_ = ResolveResetPolarity();
  reset_name_ = i_mod->GetParams()->GetResetName();
  if (reset_name_.empty()) {
    if (reset_polarity_) {
      reset_name_ = "rst";
    } else {
      reset_name_ = "rst_n";
    }
  }
  name_ = i_mod_->GetDesign()->GetParams()->GetModuleNamePrefix()
    + i_mod->GetName();
}

Module::~Module() {
  STLDeleteValues(&tables_);
}

void Module::Write(ostream &os) {
  os << "module " << GetName() << "(";
  ports_->Output(Ports::PORT_NAME, os);
  os << ");\n";
  ports_->Output(Ports::PORT_TYPE, os);
  os << "\n";

  os << "  // State decls\n"
     << tmpl_->GetContents(kStateDeclSection)
     << "  // State vars\n"
     << tmpl_->GetContents(kStateVarSection)
     << "  // Registers\n"
     << tmpl_->GetContents(kRegisterSection)
     << "  // Resources\n"
     << tmpl_->GetContents(kResourceSection)
     << "  // Insn wires\n"
     << tmpl_->GetContents(kInsnWireDeclSection)
     << "  // Insn assigns\n"
     << tmpl_->GetContents(kInsnWireValueSection);
  os << "\n";

  for (auto *tab : tables_) {
    tab->Write(os);
  }
  os << tmpl_->GetContents(kEmbeddedInstanceSection);
  os << tmpl_->GetContents(kSubModuleSection);
  os << "\nendmodule\n";
}

bool Module::GetResetPolarity() const {
  return reset_polarity_;
}

const IModule *Module::GetIModule() const {
  return i_mod_;
}

const Ports *Module::GetPorts() const {
  return ports_.get();
}

void Module::Build() {
  ports_->AddPort("clk", Port::INPUT_CLK, 0);
  ports_->AddPort(reset_name_, Port::INPUT_RESET, 0);

  for (auto *i_table : i_mod_->tables_) {
    Table *tab = new Table(i_table, ports_.get(), this, embed_,
			   tmpl_.get());
    tab->Build();
    tables_.push_back(tab);
  }

  const ChannelInfo *ci = conn_.GetConnectionInfo(i_mod_);
  if (ci != nullptr) {
    Channel::BuildChannelPorts(*ci, ports_.get());
    Channel::BuildRootWire(*ci, this);
  }
  const TaskCallInfo *ti = conn_.GetTaskCallInfo(i_mod_);
  if (ti != nullptr) {
    SubModuleTask::BuildPorts(*ti, ports_.get());
  }
  const RegConnectionInfo *ri = conn_.GetRegConnectionInfo(i_mod_);
  if (ri != nullptr) {
    SharedReg::BuildPorts(*ri, ports_.get());
    SharedReg::BuildRegWire(*ri, this);
  }
}

void Module::BuildChildModuleSection(vector<Module *> &child_mods) {
  ostream &is = tmpl_->GetStream(kSubModuleSection);
  for (auto *child_mod : child_mods) {
    // mod inst_mod(...);
    const IModule *child_imod = child_mod->GetIModule();
    is << "  " << child_mod->GetName() << " "
       << "inst_" << child_mod->GetName() << "(";
    is << "." << child_mod->GetPorts()->GetClk() << "(" << GetPorts()->GetClk() << ")";
    is << ", ." << child_mod->GetPorts()->GetReset() << "("
       << GetPorts()->GetReset() << ")";
    // Task
    const TaskCallInfo *ti = conn_.GetTaskCallInfo(child_imod);
    if (ti != nullptr) {
      SubModuleTask::BuildChildTaskWire(*ti, is);
    }
    // Channel
    const ChannelInfo *ci = conn_.GetConnectionInfo(i_mod_);
    if (ci != nullptr) {
      Channel::BuildChildChannelWire(*ci, child_imod, is);
    }
    // Registers
    const RegConnectionInfo *ri = conn_.GetRegConnectionInfo(child_imod);
    if (ri != nullptr) {
      SharedReg::BuildChildWire(*ri, is);
    }
    is << ");\n";
  }
}

bool Module::ResolveResetPolarity() {
  for (const IModule *mod = i_mod_; mod != nullptr;
       mod = mod->GetParentModule()) {
    if (mod->GetParams()->HasResetPolarity()) {
      return mod->GetParams()->GetResetPolarity();
    }
  }
  // This may return the default value.
  return i_mod_->GetDesign()->GetParams()->GetResetPolarity();
}

ModuleTemplate *Module::GetModuleTemplate() const {
  return tmpl_.get();
}

const string &Module::GetName() const {
  return name_;
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
