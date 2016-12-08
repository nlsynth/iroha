#include "writer/verilog/verilog_writer.h"

#include "design/design_util.h"
#include "iroha/logging.h"
#include "iroha/i_design.h"
#include "iroha/stl_util.h"
#include "writer/names.h"
#include "writer/verilog/embed.h"
#include "writer/verilog/internal_sram.h"
#include "writer/verilog/module.h"
#include "writer/verilog/ports.h"

namespace iroha {
namespace writer {
namespace verilog {

VerilogWriter::VerilogWriter(const IDesign *design, const Connection &conn,
			     ostream &os)
  : design_(design), conn_(conn), os_(os),
    embedded_modules_(new EmbeddedModules), with_self_clock_(false),
    names_(new Names(nullptr)) {
}

VerilogWriter::~VerilogWriter() {
}

bool VerilogWriter::Write() {
  names_->ReservePrefix("channel");
  names_->ReservePrefix("insn");
  names_->ReservePrefix("shared_reg");
  names_->ReservePrefix("st");
  names_->ReservePrefix("task");
  names_->ReservePrefix("S");

  os_ << "// Generated from " << PACKAGE << "-" << VERSION << ".\n\n";

  IModule *root = DesignUtil::GetRootModule(design_);
  if (root == nullptr) {
    LOG(ERROR) << "Failed to determine a root module";
    return false;
  }
  PrepareModulesRec(root);
  BuildModules(root);
  if (!embedded_modules_->Write(os_)) {
    LOG(ERROR) << "Failed to write embedded modules.";
    return false;
  }
  for (auto *mod : ordered_modules_) {
    mod->Write(os_);
  }
  if (!shell_module_name_.empty()) {
    WriteShellModule(modules_[root]);
  }
  STLDeleteSecondElements(&modules_);
  return true;
}

void VerilogWriter::SetShellModuleName(const string &n, bool with_self_clock) {
  shell_module_name_ = n;
  with_self_clock_ = with_self_clock;
}

Module *VerilogWriter::GetByIModule(const IModule *mod) const {
  auto it = modules_.find(mod);
  if (it != modules_.end()) {
    return it->second;
  }
  return nullptr;
}

void VerilogWriter::PrepareModulesRec(const IModule *imod) {
  vector<IModule *> children = DesignUtil::GetChildModules(imod);
  for (IModule *child : children) {
    PrepareModulesRec(child);
  }
  Module *mod = new Module(imod, this, conn_,
			   embedded_modules_.get(),
			   names_->GetNewChildNames());
  modules_[imod] = mod;
  ordered_modules_.push_back(mod);
}

void VerilogWriter::BuildModules(const IModule *imod) {
  for (auto *mod : ordered_modules_) {
    const IModule *imod = mod->GetIModule();
    IModule *iparent = imod->GetParentModule();
    if (iparent != nullptr) {
      Module *parent = modules_[iparent];
      mod->SetParentModule(parent);
    }
  }
  for (auto *mod : ordered_modules_) {
    mod->PrepareTables();
  }
  for (auto *mod : ordered_modules_) {
    mod->Build();
  }
  BuildChildModuleSection();
}

void VerilogWriter::BuildChildModuleSection() {
  for (auto *mod : ordered_modules_) {
    vector<IModule *> child_imods =
      DesignUtil::GetChildModules(mod->GetIModule());
    vector<Module *> mods;
    for (auto *imod : child_imods) {
      mods.push_back(modules_[imod]);
    }
    mod->BuildChildModuleInstSection(mods);
  }
}

void VerilogWriter::WriteShellModule(const Module *mod) {
  const Ports *ports = mod->GetPorts();
  os_ << "module " << shell_module_name_ << "(";
  if (!with_self_clock_) {
    ports->Output(Ports::PORT_NAME, os_);
  }
  os_ << ");\n";
  if (with_self_clock_) {
    WriteSelfClockGenerator(mod);
  } else {
    ports->Output(Ports::PORT_DIRECTION, os_);
  }
  string name = mod->GetName();
  os_ << "  " << name << " " << name << "_inst(";
  if (with_self_clock_) {
    WriteSelfClockConnection(mod);
  } else {
    ports->Output(Ports::PORT_CONNECTION, os_);
  }
  os_ << ");\n";
  os_ << "endmodule\n";
}

void VerilogWriter::WriteSelfClockGenerator(const Module *mod) {
  const Ports *ports = mod->GetPorts();
  const string &clk = ports->GetClk();
  const string &rst = ports->GetReset();
  bool polarity = mod->GetResetPolarity();
  os_ << "  reg " << clk << ", " <<  rst << ";\n\n"
      << "  initial begin\n"
      << "    " << clk << " <= 0;\n"
      << "    " << rst << " <= " << (polarity ? "1" : "0") << ";\n"
      << "    #15\n"
      << "    " << rst << " <= ~" << rst << ";\n"
      << "    #10000\n"
      << "    $finish;\n"
      << "  end\n\n";
  os_ << "  always begin\n"
      << "    #10 " << clk << " = ~" << clk << ";\n"
      << "  end\n\n";
}

void VerilogWriter::WriteSelfClockConnection(const Module *mod) {
  const Ports *ports = mod->GetPorts();
  const string &clk = ports->GetClk();
  const string &rst = ports->GetReset();
  os_ << "." << clk << "(" << clk << "), ." << rst << "(" << rst << ")";
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
