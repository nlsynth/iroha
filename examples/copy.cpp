#include "iroha/iroha.h"
#include "design/table_copier.h"

using namespace iroha;

int main(int argc, char **argv) {
  if (argc != 2) {
    return 0;
  }
  IDesign *design = Iroha::ReadDesignFromFile(argv[1]);
  IModule *mod = design->modules_[0];
  ITable *tab = mod->tables_[0];
  ITable *copied = TableCopier::CopyTable(tab, mod);
  mod->tables_.push_back(copied);
  DesignTool::Validate(design);
  WriterAPI *writer = Iroha::CreateWriter(design);
  writer->Write("/dev/stdout");
  return 0;
}
