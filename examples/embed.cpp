#include "example_common.h"
#include "iroha/iroha.h"

using namespace iroha;

IDesign *build_design() {
  IDesign *design = new IDesign;

  IModule *module = new IModule(design, "M");
  design->modules_.push_back(module);
  ITable *table = new ITable(module);
  module->tables_.push_back(table);
  IState *st1 = new IState(table);
  table->states_.push_back(st1);
  table->SetInitialState(st1);
  IResource *vmod =
    DesignTool::CreateEmbedResource(table, "mod_hello", "mod_hello.v");
  IInsn *insn = new IInsn(vmod);
  st1->insns_.push_back(insn);

  DesignTool::Validate(design);

  return design;
}
