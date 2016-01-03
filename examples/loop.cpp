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
  // 1: cond = counter > 10
  // 2: if (cond) goto 5: else goto 3:
  // 3: counter = counter + 1
  // 4: goto 2:
  // 5: {nothing}
  IState *st1 = new IState(table);
  IState *st2 = new IState(table);
  IState *st3 = new IState(table);
  IState *st4 = new IState(table);
  IState *st5 = new IState(table);
  table->states_.push_back(st1);
  table->states_.push_back(st2);
  table->states_.push_back(st3);
  table->states_.push_back(st4);
  table->states_.push_back(st5);

  IRegister *counter = tool->AllocRegister(table, "counter", 32);
  tool->SetRegisterInitialValue(0, counter);
  IRegister *ten = tool->AllocConstNum(table, 32, 10);
  IResource *gt = tool->GetBinOpResource(table, resource::kGt, 32);
  IRegister *cond = tool->AllocRegister(table, "cond", 0);
  IInsn *compare_insn = new IInsn(gt);
  compare_insn->inputs_.push_back(counter);
  compare_insn->inputs_.push_back(ten);
  compare_insn->outputs_.push_back(cond);
  st1->insns_.push_back(compare_insn);
  tool->AddNextState(st1, st2);
  IInsn *br = tool->AddNextState(st2, st3);
  tool->AddNextState(st2, st5);
  br->inputs_.push_back(cond);
  tool->AddNextState(st4, st1);

  IResource *adder = tool->GetBinOpResource(table, resource::kAdd, 32);
  IRegister *one = tool->AllocConstNum(table, 32, 1);
  IInsn *add_insn = new IInsn(adder);
  add_insn->inputs_.push_back(counter);
  add_insn->inputs_.push_back(one);
  add_insn->outputs_.push_back(counter);
  st3->insns_.push_back(add_insn);

  table->SetInitialState(st1);
  tool->AddNextState(st3, st4);
  tool->Validate(NULL);

  delete tool;

  return design;
}
