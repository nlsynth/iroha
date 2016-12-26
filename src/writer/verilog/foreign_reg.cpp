#include "writer/verilog/foreign_reg.h"

#include "iroha/i_design.h"
#include "writer/connection.h"
#include "writer/module_template.h"
#include "writer/verilog/insn_writer.h"
#include "writer/verilog/module.h"
#include "writer/verilog/ports.h"
#include "writer/verilog/table.h"

namespace iroha {
namespace writer {
namespace verilog {

ForeignReg::ForeignReg(const IResource &res, const Table &table)
  : Resource(res, table) {
}

void ForeignReg::BuildResource() {
}

void ForeignReg::BuildInsn(IInsn *insn, State *st) {
  if (insn->outputs_.size() == 0) {
    return;
  }
  ostream &ws = tmpl_->GetStream(kInsnWireValueSection);
  ws << "  assign " << InsnWriter::InsnOutputWireName(*insn, 0)
     << " = "
     << InsnWriter::RegisterValue(*(res_.GetForeignRegister()), tab_.GetNames())
     << ";\n";
}

void ForeignReg::BuildPorts(const RegConnectionInfo &ri, Ports *ports, Names *names) {
  for (IRegister *reg : ri.has_upward_port) {
    ports->AddPort(ForeignRegName(reg, names), Port::OUTPUT_WIRE,
		   reg->value_type_.GetWidth());
  }
  for (IRegister *reg : ri.has_downward_port) {
    ports->AddPort(ForeignRegName(reg, names), Port::INPUT,
		   reg->value_type_.GetWidth());
  }
}

void ForeignReg::BuildChildWire(const RegConnectionInfo &ri, Names *names,
				ostream &os) {
  for (IRegister *reg : ri.has_upward_port) {
    AddChildWire(reg, names, os);
  }
  for (IRegister *reg : ri.has_downward_port) {
    AddChildWire(reg, names, os);
  }
}

void ForeignReg::AddChildWire(IRegister *reg, Names *names, ostream &os) {
  string name = ForeignRegName(reg, names);
  os << ", ." << name << "(" << name << ")";
}

void ForeignReg::BuildRegWire(const RegConnectionInfo &ri, Module *module) {
  ModuleTemplate *tmpl = module->GetModuleTemplate();
  ostream &ws = tmpl->GetStream(kInsnWireValueSection);
  for (IRegister *reg : ri.is_source) {
    ws << "  wire " << Table::ValueWidthSpec(reg->value_type_)
       << ForeignRegName(reg, module->GetNames()) << ";\n";
    ws << "  assign " << ForeignRegName(reg, module->GetNames()) << " = "
       << InsnWriter::RegisterValue(*reg, module->GetNames()) << ";\n";
  }
  for (IRegister *reg : ri.has_wire) {
    ws << "  wire " << Table::ValueWidthSpec(reg->value_type_)
       << ForeignRegName(reg, module->GetNames()) << ";\n";
  }
}

  string ForeignReg::ForeignRegName(const IRegister *reg, Names *names) {
  return reg->GetTable()->GetModule()->GetName() + "_" +
    InsnWriter::RegisterValue(*reg, names);
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
