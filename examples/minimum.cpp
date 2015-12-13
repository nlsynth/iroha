#include "iroha/iroha.h"

using namespace iroha;

int main(int argc, char **argv) {
  Iroha::Init();

  IDesign *design = new IDesign;
  IModule *module = new IModule;
  design->modules_.push_back(module);
  ITable *table = new ITable;
  module->tables_.push_back(table);
  IState *st1 = new IState;
  IState *st2 = new IState;
  table->states_.push_back(st1);
  table->states_.push_back(st2);

  iroha::WriterAPI *writer = iroha::Iroha::CreateWriter(design);
  writer->Write("/dev/stdout");

  return 0;
}
