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
  IState *st2 = new IState(table);
  table->states_.push_back(st2);
  DesignTool::AddNextState(st1, st2);

  IResource *array =
    DesignTool::CreateArrayResource(table, 10, 32,
				    /* is_external */ false, /* is_ram */ true);
  IRegister *addr = DesignTool::AllocConstNum(table, 10, 0);
  IRegister *write_data = DesignTool::AllocConstNum(table, 32, 123);
  IInsn *write_insn = new IInsn(array);
  write_insn->inputs_.push_back(addr);
  write_insn->inputs_.push_back(write_data);
  st1->insns_.push_back(write_insn);
  IInsn *read_insn = new IInsn(array);
  read_insn->inputs_.push_back(addr);
  IRegister *read_data = DesignTool::AllocRegister(table, "x", 32);
  read_insn->outputs_.push_back(read_data);
  st2->insns_.push_back(read_insn);

  DesignTool::Validate(design);
  return design;
}
