#include "iroha/iroha.h"

using namespace iroha;

int main(int argc, char **argv) {
  Iroha::Init();

  IDesign *design = new IDesign;
  IModule *module = new IModule(design, "M");
  design->modules_.push_back(module);
  ITable *table = new ITable(module);
  module->tables_.push_back(table);
  IState *st1 = new IState(table);
  IState *st2 = new IState(table);
  table->states_.push_back(st1);
  table->states_.push_back(st2);

  table->SetInitialState(st1);

  DesignToolAPI *tool = Iroha::CreateDesignTool(design);
  tool->ValidateStateId(NULL);

  iroha::WriterAPI *writer = Iroha::CreateWriter(design);
  writer->Write("/dev/stdout");

  delete tool;
  delete writer;
  delete design;

  return 0;
}
