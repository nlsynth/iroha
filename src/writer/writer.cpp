#include "writer/writer.h"

#include <fstream>
#include "iroha/iroha.h"

namespace iroha {

Writer::Writer(const IDesign *design) : design_(design) {
}

void Writer::Write(const string &fn) {
  os_ = new ofstream(fn);
  for (auto *mod : design_->modules_) {
    WriteModule(mod);
  }
  delete os_;
}

void Writer::WriteModule(const IModule *mod) {
  *os_ << "(MODULE " << mod->GetName() << " ";
  for (auto *tab : mod->tables_) {
    WriteTable(tab);
  }
  *os_ << ")\n";
}

void Writer::WriteTable(const ITable *tab) {
  *os_ << "(TABLE ";
  for (auto *st : tab->states_) {
    WriteState(st);
  }
  *os_ << ")";
}

void Writer::WriteState(const IState *st) {
  *os_ << "(STATE " << st->GetId();
  for (auto *insn : st->insns_) {
    *os_ << " ";
    WriteInsn(insn);
  }
  *os_ << ")";
}

void Writer::WriteInsn(const IInsn *insn) {
  *os_ << "(INSN ";
  const IResource *res = insn->GetResource();
  *os_ << res->GetClass()->GetName();
  *os_ << " (";
  bool is_first = true;
  for (IState *st : insn->target_states_) {
    if (!is_first) {
      *os_ << " ";
    }
    is_first = false;
    *os_ << st->GetId();
  }
  *os_ << "))";
}

}  // namespace iroha
