#include "example_common.h"
#include "iroha/iroha.h"

using namespace iroha;

IDesign *build_design() {
  IDesign *design = new IDesign;
  DesignToolAPI *tool = Iroha::CreateDesignTool(design);

  IModule *module = new IModule(design, "M");
  design->modules_.push_back(module);
  ITable *table = new ITable(module);
  module->tables_.push_back(table);
  IState *st1 = new IState(table);
  table->states_.push_back(st1);
  table->SetInitialState(st1);

  IResource *array =
    tool->CreateArrayResource(table, 10, 32,
			      /* is_external */ false, /* is_ram */ true);
  (void)array;

  tool->ValidateIds(NULL);
  delete tool;
  return design;
}

int main(int argc, char **argv) {
  example_init(argc, argv);

  IDesign *design = build_design();
  OptAPI *opt = Iroha::CreateOptimizer(design);
  opt->ApplyPhase("a_phase");

  example_write(design);

  delete opt;
  delete design;
  return 0;
}
