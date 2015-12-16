#include "writer/exp_writer.h"

#include "iroha/i_design.h"

namespace iroha {

ExpWriter::ExpWriter(const IDesign *design, ostream &os)
  : design_(design), os_(os) {
}

void ExpWriter::Write() {
  for (auto *mod : design_->modules_) {
    WriteModule(mod);
  }
}

void ExpWriter::WriteModule(const IModule *mod) {
  os_ << "(MODULE " << mod->GetName() << " ";
  for (auto *tab : mod->tables_) {
    WriteTable(tab);
  }
  os_ << ")\n";
}

void ExpWriter::WriteTable(const ITable *tab) {
  os_ << "(TABLE ";
  for (auto *st : tab->states_) {
    WriteState(st);
  }
  os_ << ")";
}

void ExpWriter::WriteState(const IState *st) {
  os_ << "(STATE " << st->GetId();
  for (auto *insn : st->insns_) {
    os_ << " ";
    WriteInsn(insn);
  }
  os_ << ")";
}

void ExpWriter::WriteInsn(const IInsn *insn) {
  os_ << "(INSN ";
  const IResource *res = insn->GetResource();
  os_ << res->GetClass()->GetName();
  os_ << " (";
  bool is_first = true;
  for (IState *st : insn->target_states_) {
    if (!is_first) {
      os_ << " ";
    }
    is_first = false;
    os_ << st->GetId();
  }
  os_ << "))";
}

}  // namespace iroha
