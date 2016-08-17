#include "writer/cxx/module.h"

#include "iroha/i_design.h"
#include "writer/cxx/class_writer.h"
#include "writer/cxx/table.h"

namespace iroha {
namespace writer {
namespace cxx {

Module::Module(const IModule *i_mod) : i_mod_(i_mod) {
  class_writer_.reset(new ClassWriter(i_mod->GetName(), "ModuleBase"));
}

Module::~Module() {
}

string Module::GetName() {
  return i_mod_->GetName();
}

void Module::Build() {
  for (const ITable *tab : i_mod_->tables_) {
    tables_.push_back(unique_ptr<Table>(new Table(tab)));
  }
  for (auto &tab : tables_) {
    tab->Build();
  }
}

void Module::Write(ostream &os) {
  class_writer_->Write(os);
  for (auto &tab : tables_) {
    tab->Write(os);
  }
}

}  // namespace cxx
}  // namespace writer
}  // namespace iroha
