#include "example_common.h"
#include "iroha/iroha.h"

using namespace iroha;

IModule *create_module(DesignTool *tool, const string &name) {
  IModule *mod = new IModule(tool->GetDesign(), name);
  ITable *table = new ITable(mod);
  mod->tables_.push_back(table);
  IState *st1 = new IState(table);
  table->states_.push_back(st1);
  table->SetInitialState(st1);
  return mod;
}

IModule *create_sub_module(DesignTool *tool) {
  IModule *mod = create_module(tool, "M_sub");
  ITable *tab = mod->tables_[0];
  IResource *r = tool->GetResource(tab, resource::kChannelRead);
  tool->GetDesign()->channels_[0]->SetReader(r);
  IInsn *insn = new IInsn(r);
  IState *st1 = tab->states_[0];
  st1->insns_.push_back(insn);
  // Write to external.
  IResource *ext_write = tool->GetResource(tab, resource::kChannelWrite);
  tool->GetDesign()->channels_[1]->SetWriter(ext_write);
  IInsn *write_insn = new IInsn(ext_write);
  st1->insns_.push_back(write_insn);
  return mod;
}

IModule *create_root_module(DesignTool *tool, IModule *sub_module) {
  IModule *mod = create_module(tool, "M_top");
  ITable *tab = mod->tables_[0];
  IResource *w = tool->GetResource(tab, resource::kChannelWrite);
  tool->GetDesign()->channels_[0]->SetWriter(w);
  IInsn *insn = new IInsn(w);
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
  DesignTool *tool = Iroha::CreateDesignTool(design);
  IModule *sub = create_sub_module(tool);
  IModule *root = create_root_module(tool, sub);
  sub->SetParentModule(root);
  design->modules_.push_back(root);
  design->modules_.push_back(sub);
  tool->Validate(NULL);
  delete tool;
  return design;
}
