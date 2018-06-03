#include "writer/verilog/module.h"

#include "design/design_util.h"
#include "iroha/i_design.h"
#include "iroha/resource_params.h"
#include "iroha/stl_util.h"
#include "writer/connection.h"
#include "writer/module_template.h"
#include "writer/verilog/dataflow_table.h"
#include "writer/verilog/foreign_reg.h"
#include "writer/verilog/ports.h"
#include "writer/verilog/table.h"
#include "writer/verilog/verilog_writer.h"

namespace iroha {
namespace writer {
namespace verilog {

Module::Module(const IModule *i_mod, const VerilogWriter *writer,
	       const Connection &conn,
	       EmbeddedModules *embed, Names *names)
  : i_mod_(i_mod), writer_(writer), conn_(conn), embed_(embed), names_(names),
    parent_(nullptr) {
  tmpl_.reset(new ModuleTemplate);
  ports_.reset(new Ports);
  reset_name_ = i_mod->GetParams()->GetResetName();
  name_ = i_mod_->GetDesign()->GetParams()->GetModuleNamePrefix()
    + i_mod->GetName();
}

Module::~Module() {
  STLDeleteValues(&tables_);
}

void Module::Write(ostream &os) {
  os << "\n// Module " << i_mod_->GetId() << ";\n"
     << "module " << GetName() << "(";
  ports_->Output(Ports::PORT_NAME, os);
  os << ");\n";
  ports_->Output(Ports::PORT_TYPE, os);
  os << "\n";

  os << "  // State decls\n"
     << tmpl_->GetContents(kStateDeclSection)
     << "  // Shared wires\n"
     << tmpl_->GetContents(kSharedWireSection)
     << "  // Shared insn wires\n"
     << tmpl_->GetContents(kInsnWireDeclSection)
     << "  // Shared insn assigns\n"
     << tmpl_->GetContents(kInsnWireValueSection);
  os << "\n";

  for (auto *tab : tables_) {
    string s = tab->RegisterSectionContents();
    if (!s.empty()) {
      os << "  // Registers for table " << tab->GetITable()->GetId() << "\n";
      os << s << "\n";
    }
    s = tab->InsnWireDeclSectionContents();
    if (!s.empty()) {
      os << "  // Insn wires for table " << tab->GetITable()->GetId() << "\n";
      os << s << "\n";
    }
    s = tab->InsnWireValueSectionContents();
    if (!s.empty()) {
      os << "  // Insn values for table " << tab->GetITable()->GetId() << "\n";
      os << s << "\n";
    }
    s = tab->ResourceSectionContents();
    if (!s.empty()) {
      os << "  // Resources for table " << tab->GetITable()->GetId() << "\n";
      os << s << "\n";
    }
    s = tab->ResourceValueSectionContents();
    if (!s.empty()) {
      os << "  // Resources value for table " << tab->GetITable()->GetId() << "\n";
      os << s << "\n";
    }
  }

  for (auto *tab : tables_) {
    tab->Write(os);
  }
  os << tmpl_->GetContents(kEmbeddedInstanceSection);
  for (Module *child : child_modules_) {
    // mod inst_mod(...);
    const IModule *child_imod = child->GetIModule();
    string prefix = i_mod_->GetDesign()->GetParams()->GetModuleNamePrefix();
    os << "  " << prefix << child_imod->GetName() << " "
       << "inst_" << child_imod->GetName() << "(";
    os << ChildModuleInstSectionContents(child, false);
    os << ");\n";
  }
  os << "\nendmodule // " << GetName() << "\n";
}

bool Module::GetResetPolarity() const {
  return writer_->GetResetPolarity();
}

Module *Module::GetParentModule() const {
  return parent_;
}

void Module::SetParentModule(Module *parent) {
  parent_ = parent;
}

const IModule *Module::GetIModule() const {
  return i_mod_;
}

Ports *Module::GetPorts() const {
  return ports_.get();
}

void Module::PrepareTables() {
  for (auto *i_table : i_mod_->tables_) {
    Table *tab;
    IInsn *insn = DesignUtil::FindDataFlowInInsn(i_table);
    if (insn) {
      tab = new DataFlowTable(i_table, ports_.get(), this, embed_,
			      names_, tmpl_.get());
    } else {
      tab = new Table(i_table, ports_.get(), this, embed_, names_,
		      tmpl_.get());
    }
    tables_.push_back(tab);
  }
  for (Table *tab : tables_) {
    tab->CollectNames();
  }
}

void Module::Build() {
  if (reset_name_.empty()) {
    if (GetResetPolarity()) {
      reset_name_ = "rst";
    } else {
      reset_name_ = "rst_n";
    }
  }

  ports_->AddPort("clk", Port::INPUT_CLK, 0);
  ports_->AddPort(reset_name_, Port::INPUT_RESET, 0);

  for (Table *tab : tables_) {
    tab->Build();
  }

  const RegConnectionInfo *ri = conn_.GetRegConnectionInfo(i_mod_);
  if (ri != nullptr) {
    ForeignReg::BuildPorts(*ri, ports_.get(), names_);
    ForeignReg::BuildRegWire(*ri, this);
  }
}

void Module::BuildChildModuleInstSection(vector<Module *> &child_mods) {
  child_modules_ = child_mods;

  // Builds port connections.
  for (auto *child_mod : child_modules_) {
    string current_content = ChildModuleInstSectionContents(child_mod, true);
    ostream &is = ChildModuleInstSectionStream(child_mod);
    is << "." << child_mod->GetPorts()->GetClk()
       << "(" << GetPorts()->GetClk() << ")";
    is << ", ." << child_mod->GetPorts()->GetReset()
       << "(" << GetPorts()->GetReset() << ")";
    is << current_content;
    // Registers
    const IModule *child_imod = child_mod->GetIModule();
    const RegConnectionInfo *ri = conn_.GetRegConnectionInfo(child_imod);
    if (ri != nullptr) {
      ForeignReg::BuildChildWire(*ri, child_mod->GetNames(), is);
    }
  }
}

ostream &Module::ChildModuleInstSectionStream(Module *child) const {
  return tmpl_->GetStream(kSubModuleSection +
			  Util::Itoa(child->GetIModule()->GetId()));
}

string Module::ChildModuleInstSectionContents(Module *child, bool clear) const {
  string section = kSubModuleSection +
    Util::Itoa(child->GetIModule()->GetId());
  string s = tmpl_->GetContents(section);
  if (clear) {
    tmpl_->Clear(section);
  }
  return s;
}

ModuleTemplate *Module::GetModuleTemplate() const {
  return tmpl_.get();
}

const string &Module::GetName() const {
  return name_;
}

const Connection &Module::GetConnection() const {
  return conn_;
}

Names *Module::GetNames() const {
  return names_;
}

Module *Module::GetByIModule(const IModule *mod) const {
  return writer_->GetByIModule(mod);
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
