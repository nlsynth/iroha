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
  IState *st2 = new IState(table);
  IState *st3 = new IState(table);
  table->states_.push_back(st1);
  table->states_.push_back(st2);
  table->states_.push_back(st3);

  IRegister *my_reg = DesignTool::AllocRegister(table, "my_reg", 32);

  IResource *ext_input = DesignTool::GetOneResource(table, resource::kExtInput);
  ResourceParams *params_input = ext_input->GetParams();
  params_input->SetExtInputPort("data_in", 32);
  IInsn *input_insn = new IInsn(ext_input);
  input_insn->outputs_.push_back(my_reg);
  st1->insns_.push_back(input_insn);

  IResource *ext_output = DesignTool::GetOneResource(table, resource::kExtOutput);
  ResourceParams *params_output = ext_output->GetParams();
  params_output->SetExtOutputPort("data_out", 32);
  IInsn *output_insn = new IInsn(ext_output);
  output_insn->inputs_.push_back(my_reg);
  st2->insns_.push_back(output_insn);

  table->SetInitialState(st1);
  DesignTool::AddNextState(st1, st2);
  DesignTool::AddNextState(st2, st3);
  DesignTool::Validate(design);

  return design;
}
