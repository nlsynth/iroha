#include "writer/verilog/module.h"

#include "iroha/i_design.h"
#include "iroha/resource_params.h"
#include "iroha/stl_util.h"
#include "writer/module_template.h"
#include "writer/verilog/ports.h"
#include "writer/verilog/table.h"

namespace iroha {
namespace verilog {

Module::Module(const IModule *i_mod) : i_mod_(i_mod) {
  tmpl_.reset(new ModuleTemplate);
  ports_.reset(new Ports);
  reset_polarity_ = i_mod_->GetDesign()->GetParams()->GetResetPolarity();
}

Module::~Module() {
  STLDeleteValues(&tables_);
}

void Module::Write(ostream &os) {
  Build();
  os << "module " << i_mod_->GetName() << "(";
  ports_->Output(Ports::PORT_NAME, os);
  os << ");\n";
  ports_->Output(Ports::PORT_TYPE, os);
  os << "\n";

  os << tmpl_->GetContents(kStateDeclSection);
  os << tmpl_->GetContents(kStateVarSection);
  os << tmpl_->GetContents(kRegisterSection);
  os << tmpl_->GetContents(kResourceSection);
  os << "\n";

  for (auto *tab : tables_) {
    tab->Write(os);
  }
  os << "endmodule\n";
}

bool Module::GetResetPolarity() const {
  return reset_polarity_;
}

void Module::Build() {
  ports_->AddPort("clk", Port::INPUT_CLK, 0);
  if (reset_polarity_) {
    ports_->AddPort("rst", Port::INPUT_RESET, 0);
  } else {
    ports_->AddPort("rst_n", Port::INPUT_RESET, 0);
  }

  int nth = 0;
  for (auto *i_table : i_mod_->tables_) {
    Table *tab = new Table(i_table, ports_.get(), this, tmpl_.get(), nth);
    tab->Build();
    tables_.push_back(tab);
    ++nth;
  }
}

}  // namespace verilog
}  // namespace iroha
