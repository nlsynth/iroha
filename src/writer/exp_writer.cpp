#include "writer/exp_writer.h"

#include "iroha/i_design.h"

namespace iroha {

ExpWriter::ExpWriter(const IDesign *design, ostream &os)
  : design_(design), os_(os) {
}

void ExpWriter::Write() {
  for (auto *mod : design_->modules_) {
    WriteModule(mod);
  }
}

void ExpWriter::WriteModule(const IModule *mod) {
  os_ << "(MODULE " << mod->GetName() << "\n";
  for (auto *tab : mod->tables_) {
    WriteTable(tab);
  }
  os_ << ")\n";
}

void ExpWriter::WriteTable(const ITable *tab) {
  os_ << "  (TABLE\n";
  WriteRegisters(tab);
  for (auto *st : tab->states_) {
    WriteState(st);
  }
  os_ << "  )\n";
}

void ExpWriter::WriteRegisters(const ITable *tab) {
  os_ << "    (REGISTERS\n";
  for (auto *reg : tab->registers_) {
    os_ << "      (REGISTER " << reg->GetName() << "\n";
    os_ << "        (";
    if (reg->is_const_) {
      os_ << "CONST";
    } else if (reg->state_local_) {
      os_ << "WIRE";
    } else {
      os_ << "REG";
    }
    os_ << ")\n";
    os_ << "      )\n";
  }
  os_ << "    )\n";
}

void ExpWriter::WriteState(const IState *st) {
  os_ << "    (STATE " << st->GetId() << "\n";
  for (auto *insn : st->insns_) {
    WriteInsn(insn);
  }
  os_ << "    )\n";
}

void ExpWriter::WriteInsn(const IInsn *insn) {
  os_ << "      (INSN ";
  const IResource *res = insn->GetResource();
  os_ << res->GetClass()->GetName();
  // Targets.
  os_ << " (";
  bool is_first = true;
  for (IState *st : insn->target_states_) {
    if (!is_first) {
      os_ << " ";
    }
    is_first = false;
    os_ << st->GetId();
  }
  os_ << ")";
  // Outputs.
  WriteInsnParams(insn->outputs_);
  // Inputs.
  WriteInsnParams(insn->inputs_);
  os_ << ")\n";
}

void ExpWriter::WriteInsnParams(const vector<IRegister *> &regs) {
  os_ << " (";
  bool is_first = true;
  for (auto &reg : regs) {
    if (!is_first) {
      os_ << " ";
    }
    os_ << reg->GetName();
    is_first = false;
  }
  os_ << ")";
    
}
  
}  // namespace iroha
