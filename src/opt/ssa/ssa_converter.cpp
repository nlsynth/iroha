#include "opt/ssa/ssa_converter.h"

#include "design/design_tool.h"
#include "design/design_util.h"
#include "iroha/i_design.h"
#include "iroha/logging.h"
#include "iroha/resource_class.h"
#include "opt/data_flow.h"
#include "opt/dominator_tree.h"
#include "opt/opt_util.h"
#include "opt/ssa/phi_builder.h"
#include "opt/ssa/phi_injector.h"

namespace iroha {
namespace opt {
namespace ssa {

SSAConverter::SSAConverter(ITable *table, OptimizerLog *opt_log)
    : table_(table), opt_log_(opt_log) {}

SSAConverter::~SSAConverter() {}

void SSAConverter::Perform() {
  InjectInitialValueAssigns();
  // PhiInjector injects just phi insns and their output register.
  // PhiBuilder updates affected registers.
  PhiInjector injector(table_, opt_log_);
  injector.Perform();

  PhiBuilder phi_builder(table_, opt_log_);
  phi_builder.Perform();
}

void SSAConverter::InjectInitialValueAssigns() {
  // Change initial value of IRegister to explicit value assign in order
  // not to violate SSA premise.
  vector<IRegister *> new_regs;
  for (IRegister *reg : table_->registers_) {
    if (reg->IsConst()) {
      continue;
    }
    if (!reg->HasInitialValue()) {
      continue;
    }
    IRegister *ini_reg = InjectInitialValueAssignForReg(reg);
    new_regs.push_back(ini_reg);
  }
  for (IRegister *reg : new_regs) {
    table_->registers_.push_back(reg);
  }
}

IRegister *SSAConverter::InjectInitialValueAssignForReg(IRegister *reg) {
  IState *initial_st = table_->GetInitialState();
  IResource *assign = DesignTool::GetOneResource(table_, resource::kSet);
  IInsn *assign_insn = new IInsn(assign);
  IRegister *ini_reg = new IRegister(table_, reg->GetName() + "_ini");
  Numeric n = reg->GetInitialValue();
  ini_reg->SetInitialValue(n);
  ini_reg->SetConst(true);
  assign_insn->inputs_.push_back(ini_reg);
  assign_insn->outputs_.push_back(reg);
  initial_st->insns_.push_back(assign_insn);
  reg->ClearInitialValue();
  for (IInsn *insn : initial_st->insns_) {
    for (int i = 0; i < insn->inputs_.size(); ++i) {
      if (insn->inputs_[i] == reg) {
        insn->inputs_[i] = ini_reg;
      }
    }
  }
  return ini_reg;
}

}  // namespace ssa
}  // namespace opt
}  // namespace iroha
