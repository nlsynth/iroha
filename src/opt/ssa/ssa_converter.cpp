#include "opt/ssa/ssa_converter.h"

#include "design/design_tool.h"
#include "design/design_util.h"
#include "iroha/i_design.h"
#include "iroha/logging.h"
#include "iroha/resource_class.h"
#include "opt/bb_set.h"
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
  IResource *assign = DesignTool::GetOneResource(table_, resource::kSet);
  IState *initial_st = table_->GetInitialState();
  vector<IRegister *> new_regs;
  for (IRegister *reg : table_->registers_) {
    if (reg->IsConst()) {
      continue;
    }
    if (!reg->HasInitialValue()) {
      continue;
    }
    IInsn *insn = new IInsn(assign);
    IRegister *ini_reg = new IRegister(table_, reg->GetName() + "_ini");
    Numeric n = reg->GetInitialValue();
    ini_reg->SetInitialValue(n);
    ini_reg->SetConst(true);
    new_regs.push_back(ini_reg);
    insn->inputs_.push_back(ini_reg);
    insn->outputs_.push_back(reg);
    initial_st->insns_.push_back(insn);
    reg->ClearInitialValue();
  }
  for (IRegister *reg : new_regs) {
    table_->registers_.push_back(reg);
  }
}

}  // namespace ssa
}  // namespace opt
}  // namespace iroha
