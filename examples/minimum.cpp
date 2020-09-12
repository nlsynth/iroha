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
  table->states_.push_back(st1);
  table->states_.push_back(st2);

  IResource *assign = DesignTool::GetOneResource(table, resource::kSet);
  IInsn *insn = new IInsn(assign);
  IRegister *dst = DesignTool::AllocRegister(table, "dst_reg", 32);
  IRegister *src = DesignTool::AllocConstNum(table, 32, 123);
  insn->inputs_.push_back(src);
  insn->outputs_.push_back(dst);

  st1->insns_.push_back(insn);

  table->SetInitialState(st1);
  DesignTool::AddNextState(st1, st2);
  DesignTool::Validate(design);

  return design;
}

int main(int argc, char **argv) {
  Iroha::Init();

  IDesign *design = build_design();

  OptAPI *opt = Iroha::CreateOptimizer(design);
  opt->ApplyPass("a_phase");

  iroha::WriterAPI *writer = Iroha::CreateWriter(design);
  // Use S-Expression.
  writer->SetLanguage("");
  writer->Write("/dev/stdout");

  writer->SetLanguage("verilog");
  writer->Write("/dev/stdout");

  delete design;

  return 0;
}
