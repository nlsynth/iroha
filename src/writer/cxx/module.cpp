#include "writer/cxx/module.h"

#include "iroha/i_design.h"
#include "iroha/stl_util.h"
#include "writer/cxx/class_writer.h"
#include "writer/cxx/table.h"

#include <sstream>

using std::ostringstream;

namespace iroha {
namespace writer {
namespace cxx {

Module::Module(const IModule *i_mod) : i_mod_(i_mod) {
  class_writer_.reset(new ClassWriter(i_mod->GetName(), "ModuleBase"));
}

Module::~Module() {
  STLDeleteValues(&tables_);
}

string Module::GetName() {
  return i_mod_->GetName();
}

void Module::Build() {
  for (const ITable *tab : i_mod_->tables_) {
    tables_.push_back(new Table(tab));
  }
  for (auto &tab : tables_) {
    tab->Build();
  }
  BuildConstructor();
}

void Module::BuildConstructor() {
  const string &name = class_writer_->GetName();
  ClassMember *cm = class_writer_->AddMethod(name, "");
  ostringstream os;
  for (Table *tab : tables_) {
    os << "    tabs_.push_back(new " << tab->GetTableName() << "());\n";
  }
  cm->body_ = os.str();
}

void Module::Write(ostream &os) {
  for (auto &tab : tables_) {
    tab->Write(os);
  }
  class_writer_->Write(os);
}

}  // namespace cxx
}  // namespace writer
}  // namespace iroha
