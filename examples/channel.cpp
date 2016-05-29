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
  IResource *r = DesignTool::GetResource(tab, resource::kChannelRead);
  design->channels_[0]->SetReader(r);
  IInsn *read_insn = new IInsn(r);
  IState *st1 = tab->states_[0];
  IRegister *reg = DesignTool::AllocRegister(tab, "tmp_reg", 32);
  read_insn->outputs_.push_back(reg);
  st1->insns_.push_back(read_insn);
  // Write to external.
  IResource *ext_write = DesignTool::GetResource(tab, resource::kChannelWrite);
  design->channels_[1]->SetWriter(ext_write);
  IInsn *write_insn = new IInsn(ext_write);
  write_insn->inputs_.push_back(reg);
  st1->insns_.push_back(write_insn);
  return mod;
}

IModule *create_root_module(IModule *sub_module) {
  IModule *mod = create_module(sub_module->GetDesign(), "M_top");
  ITable *tab = mod->tables_[0];
  IResource *w = DesignTool::GetResource(tab, resource::kChannelWrite);
  sub_module->GetDesign()->channels_[0]->SetWriter(w);
  IInsn *insn = new IInsn(w);
  IRegister *write_data = DesignTool::AllocConstNum(tab, 32, 123);
  insn->inputs_.push_back(write_data);
  IState *st1 = tab->states_[0];
  st1->insns_.push_back(insn);
  return mod;
}

IDesign *build_design() {
  IDesign *design = new IDesign;
  IChannel *channel = new IChannel(design);
  design->channels_.push_back(channel);
  IChannel *ext_channel = new IChannel(design);
  design->channels_.push_back(ext_channel);
  IModule *sub = create_sub_module(design);
  IModule *root = create_root_module(sub);
  sub->SetParentModule(root);
  design->modules_.push_back(root);
  design->modules_.push_back(sub);
  DesignTool::Validate(design);
  return design;
}
