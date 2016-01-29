#include "example_common.h"
#include "iroha/iroha.h"

using namespace iroha;

IModule *create_module(IDesign *design, const string &name) {
  IModule *mod = new IModule(design, name);
  ITable *table = new ITable(mod);
  mod->tables_.push_back(table);
  IState *st1 = new IState(table);
  table->states_.push_back(st1);
  table->SetInitialState(st1);
  return mod;
}

IModule *create_sub_module(IDesign *design) {
  IModule *mod = create_module(design, "M_sub");
  ITable *tab = mod->tables_[0];
  IResource *res = DesignTool::CreateTaskResource(tab);
  IInsn *insn = new IInsn(res);
  IState *st1 = tab->states_[0];
  st1->insns_.push_back(insn);
  return mod;
}

IModule *create_root_module(IModule *sub_module) {
  IModule *mod = create_module(sub_module->GetDesign(), "M_top");
  ITable *tab = mod->tables_[0];
  IResource *res =
    DesignTool::CreateSubModuleTaskCallResource(tab, sub_module->tables_[0]);
  IInsn *insn = new IInsn(res);
  IState *st1 = tab->states_[0];
  st1->insns_.push_back(insn);
  return mod;
}

IDesign *build_design() {
  IDesign *design = new IDesign;
  IModule *sub = create_sub_module(design);
  IModule *root = create_root_module(sub);
  sub->SetParentModule(root);
  design->modules_.push_back(root);
  design->modules_.push_back(sub);
  DesignTool::Validate(design);
  return design;
}
