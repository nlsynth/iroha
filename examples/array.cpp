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
  IState *st2 = new IState(table);
  table->states_.push_back(st2);
  tool->AddNextState(st1, st2);

  IResource *array =
    tool->CreateArrayResource(table, 10, 32,
			      /* is_external */ false, /* is_ram */ true);
  IRegister *addr = tool->AllocConstNum(table, 10, 0);
  IRegister *write_data = tool->AllocConstNum(table, 32, 123);
  IInsn *write_insn = new IInsn(array);
  write_insn->inputs_.push_back(addr);
  write_insn->inputs_.push_back(write_data);
  st1->insns_.push_back(write_insn);
  IInsn *read_insn = new IInsn(array);
  read_insn->inputs_.push_back(addr);
  IRegister *read_data = tool->AllocRegister(table, "x", 32);
  read_insn->outputs_.push_back(read_data);
  st2->insns_.push_back(read_insn);

  tool->Validate(NULL);
  delete tool;
  return design;
}
