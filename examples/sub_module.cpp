#include "example_common.h"
#include "iroha/iroha.h"

using namespace iroha;

IModule *create_module(DesignToolAPI *tool, const string &name) {
  IModule *mod = new IModule(tool->GetDesign(), name);
  ITable *table = new ITable(mod);
  mod->tables_.push_back(table);
  IState *st1 = new IState(table);
  table->states_.push_back(st1);
  table->SetInitialState(st1);
  return mod;
}

IModule *create_sub_module(DesignToolAPI *tool) {
  IModule *mod = create_module(tool, "M_sub");
  return mod;
}

IModule *create_root_module(DesignToolAPI *tool, IModule *sub_module) {
  IModule *mod = create_module(tool, "M_top");
  ITable *tab = mod->tables_[0];
  IResource *res = tool->CreateSubModuleTaskResource(tab, sub_module);
  IState *st1 = tab->states_[0];
  IInsn *insn = new IInsn(res);
  st1->insns_.push_back(insn);
  return mod;
}

IDesign *build_design() {
  IDesign *design = new IDesign;
  DesignToolAPI *tool = Iroha::CreateDesignTool(design);
  IModule *sub = create_sub_module(tool);
  IModule *root = create_root_module(tool, sub);
  sub->SetParentModule(root);
  design->modules_.push_back(root);
  design->modules_.push_back(sub);
  tool->ValidateIds(NULL);
  delete tool;
  return design;
}
