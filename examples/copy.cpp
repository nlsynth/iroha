#include "iroha/iroha.h"
#include "design/module_copier.h"

using namespace iroha;

int main(int argc, char **argv) {
  if (argc != 2) {
    return 0;
  }
  IDesign *ext_design = Iroha::ReadDesignFromFile(argv[1]);
  IDesign *new_design = new IDesign;
  IModule *new_mod = new IModule(new_design, "M");
  new_design->modules_.push_back(new_mod);
  ModuleCopier::CopyModule(ext_design->modules_[0], new_mod);
  DesignTool::Validate(new_design);
  WriterAPI *writer = Iroha::CreateWriter(new_design);
  writer->Write("/dev/stdout");
  return 0;
}
