#include "writer/dot_writer.h"

#include "iroha/dot.h"
#include "iroha/i_design.h"

namespace iroha {
namespace writer {

DotWriter::DotWriter(const IDesign *design, ostream &os)
  : design_(design), os_(os) {
}

DotWriter::~DotWriter() {
}

void DotWriter::Write() {
  dot_.reset(new Dot);
  for (auto *mod : design_->modules_) {
    WriteModule(*mod);
  }
  dot_->Output(os_);
}

void DotWriter::WriteModule(const IModule &mod) {
  Cluster *cl = dot_->GetCluster(mod.GetName());
  for (auto *tab : mod.tables_) {
    Node *n = dot_->GetNode(TableName(*tab));
    n->SetCluster(cl);
  }
}

string DotWriter::TableName(const ITable &tab) {
  return "tab_" + Util::Itoa(tab.GetModule()->GetId())
    + "_" + Util::Itoa(tab.GetId());
}

}  // namespace writer
}  // namespace iroha
