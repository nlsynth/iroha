#include "writer/cxx/resource.h"

#include "iroha/i_design.h"
#include "iroha/logging.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"

namespace iroha {
namespace writer {
namespace cxx {

void Resource::WriteInsn(IInsn *insn, ostream &os) {
  IResource *res = insn->GetResource();
  auto *rc = res->GetClass();
  if (resource::IsExclusiveBinOp(*rc) ||
      resource::IsLightBinOp(*rc)) {
    WriteBinOp(insn, os);
  }
  if (resource::IsLightUniOp(*rc)) {
    WriteUniOp(insn, os);
  }
  if (resource::IsBitShiftOp(*rc)) {
    WriteBitShift(insn, os);
  }
  if (rc->GetName() == resource::kPrint) {
    WritePrint(insn, os);
  }
  if (rc->GetName() == resource::kAssert) {
    WriteAssert(insn, os);
  }
  if (resource::IsMapped(*rc)) {
    WriteMapped(insn, os);
  }
}

void Resource::WriteBinOp(IInsn *insn, ostream &os) {
  IResource *res = insn->GetResource();
  const string &res_name = res->GetClass()->GetName();
  string op;
  if (res_name == resource::kGt) {
    op = ">";
  } else if (res_name == resource::kAdd) {
    op = "+";
  } else if (res_name == resource::kSub) {
    op = "-";
  } else if (res_name == resource::kMul) {
    op = "*";
  } else if (res_name == resource::kEq) {
    op = "==";
  } else if (res_name == resource::kGte) {
    op = ">=";
  } else if (res_name == resource::kBitAnd) {
    op = "&";
  } else if (res_name == resource::kBitOr) {
    op = "|";
  } else if (res_name == resource::kBitXor) {
    op = "^";
  } else {
    LOG(FATAL) << "Unknown binop: " << res_name;
  }
  os << "    " << insn->outputs_[0]->GetName() << " = "
     << RegValue(insn->inputs_[0]) << " " << op << " "
     << RegValue(insn->inputs_[1]) << ";\n";
}

void Resource::WriteUniOp(IInsn *insn, ostream &os) {
  IResource *res = insn->GetResource();
  const string &res_name = res->GetClass()->GetName();
  string op;
  if (res_name == resource::kBitInv) {
    op = "~";
  } else {
    LOG(FATAL) << "Unknown uniop: " << res_name;
  }
  os << "    " << insn->outputs_[0]->GetName() << " = "
     << op << RegValue(insn->inputs_[0]) << ";\n";
}

void Resource::WriteBitShift(IInsn *insn, ostream &os) {
  IResource *res = insn->GetResource();
  const string &res_name = res->GetClass()->GetName();
  if (res_name != resource::kShift) {
    return;
  }
  bool is_left = (insn->GetOperand() == "left");
  string op = is_left ? "<<" : ">>";
  const IValue &value = insn->inputs_[1]->GetInitialValue();
  int amount = value.value_;
  os << "    " << insn->outputs_[0]->GetName() << " = "
     << RegValue(insn->inputs_[0]) << " " << op << " "
     << amount << ";\n";
}

string Resource::RegValue(IRegister *reg) {
  if (reg->IsConst()) {
    return Util::Itoa(reg->GetInitialValue().value_);
  }
  return reg->GetName();
}

void Resource::WritePrint(IInsn *insn, ostream &os) {
  for (IRegister *reg : insn->inputs_) {
    os << "    cout << \"print: \" << " << RegValue(reg) << " << \"\\n\";\n";
  }
}

void Resource::WriteAssert(IInsn *insn, ostream &os) {
  for (IRegister *reg : insn->inputs_) {
    os << "    if (!" << RegValue(reg) << ") {\n"
       << "      cout << \"ASSERTION FAILURE\\n\";\n"
       << "    }\n";
  }
}

void Resource::WriteMapped(IInsn *insn, ostream &os) {
  IResource *res = insn->GetResource();
  auto *params = res->GetParams();
  if (params->GetMappedName() == "mem") {
    if (insn->GetOperand() == "sram_write") {
      os << "    mem_->Write(" << RegValue(insn->inputs_[0]) << ", "
	 << RegValue(insn->inputs_[1]) << ");\n";
    }
    if (insn->GetOperand() == "sram_read_address") {
      os << "    mem_->ReadAddr(" << RegValue(insn->inputs_[0]) << ");\n";
    }
    if (insn->GetOperand() == "sram_read_data") {
      os << "    " << insn->outputs_[0]->GetName() << " = mem_->ReadData();\n";
    }
  }
}

}  // namespace cxx
}  // namespace writer
}  // namespace iroha
