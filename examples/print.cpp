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
  IResource *pr = tool->GetResource(table, resource::kPrint);
  IResource *as = tool->GetResource(table, resource::kAssert);
  IState *st1 = new IState(table);
  IState *st2 = new IState(table);
  table->states_.push_back(st1);
  table->states_.push_back(st2);
  IInsn *pr_insn = new IInsn(pr);
  IRegister *src = tool->AllocConstNum(table, 32, 123);
  pr_insn->inputs_.push_back(src);
  IInsn *as_insn = new IInsn(as);
  IRegister *t = tool->AllocConstNum(table, 0, 1);
  as_insn->inputs_.push_back(t);
  st1->insns_.push_back(pr_insn);
  st1->insns_.push_back(as_insn);
  table->SetInitialState(st1);
  tool->AddNextState(st1, st2);
  tool->Validate(NULL);

  delete tool;
  return design;
}
