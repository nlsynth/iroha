#include "writer/cxx/table.h"

#include "iroha/i_design.h"
#include "writer/cxx/class_writer.h"

namespace iroha {
namespace writer {
namespace cxx {

Table::Table(const ITable *i_tab) : i_tab_(i_tab) {
  string name = i_tab->GetModule()->GetName()
    + "_" + Util::Itoa(i_tab->GetId());
  class_writer_.reset(new ClassWriter(name, "TableBase"));
}

Table::~Table() {
}

void Table::Build() {
}

void Table::Write(ostream &os) {
  class_writer_->Write(os);
}

}  // namespace cxx
}  // namespace writer
}  // namespace iroha
