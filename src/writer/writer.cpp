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
  *os_ << "(MODULE MOD ";
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
  *os_ << "(STATE)";
}

}  // namespace iroha
