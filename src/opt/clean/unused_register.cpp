#include "opt/clean/unused_register.h"

#include "iroha/i_design.h"

namespace iroha {
namespace opt {
namespace clean {

CleanUnusedRegPhase::~CleanUnusedRegPhase() {
}

Phase *CleanUnusedRegPhase::Create() {
  return new CleanUnusedRegPhase();
}

bool CleanUnusedRegPhase::ApplyForTable(const string &key, ITable *table) {
  set<IRegister *> regs;
  for (IState *st : table->states_) {
    for (IInsn *insn : st->insns_) {
      for (IRegister *reg : insn->inputs_) {
	regs.insert(reg);
      }
      for (IRegister *reg : insn->outputs_) {
	regs.insert(reg);
      }
    }
  }
  vector<IRegister *> live_regs;
  for (IRegister *reg : table->registers_) {
    if (regs.find(reg) != regs.end()) {
      live_regs.push_back(reg);
    }
  }
  table->registers_ = live_regs;
  return true;
}

}  // namespace clean
}  // namespace opt
}  // namespace iroha
