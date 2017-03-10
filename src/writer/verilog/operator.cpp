#include "writer/verilog/operator.h"

#include "iroha/insn_operands.h"
#include "iroha/logging.h"
#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "writer/module_template.h"
#include "writer/verilog/insn_writer.h"
#include "writer/verilog/module.h"
#include "writer/verilog/table.h"

namespace iroha {
namespace writer {
namespace verilog {

Operator::Operator(const IResource &res, const Table &table)
  : Resource(res, table) {
}

void Operator::BuildResource() {
  auto *klass = res_.GetClass();
  if (resource::IsExclusiveBinOp(*klass)) {
    BuildExclusiveBinOp();
  }
}

void Operator::BuildInsn(IInsn *insn, State *st) {
  auto *klass = res_.GetClass();
  if (resource::IsLightBinOp(*klass)) {
    BuildLightBinOpInsn(insn);
  } else if (resource::IsLightUniOp(*klass)) {
    BuildLightUniOpInsn(insn);
  } else if (resource::IsExclusiveBinOp(*klass)) {
    BuildExclusiveBinOpInsn(insn);
  } else if (resource::IsBitShiftOp(*klass)) {
    BuildBitShiftOpInsn(insn);
  } else if (resource::IsBitSel(*klass)) {
    BuildBitSelInsn(insn);
  } else if (resource::IsBitConcat(*klass)) {
    BuildBitConcatInsn(insn);
  } else if (resource::IsSelect(*klass)) {
    BuildSelectInsn(insn);
  }
}

void Operator::BuildExclusiveBinOp() {
  ostream &rs = tmpl_->GetStream(kResourceSection);
  const string &res_name = res_.GetClass()->GetName();
  rs << "  // " << res_name << ":" << res_.GetId() << "\n";
  map<IState *, IInsn *> callers;
  CollectResourceCallers("", &callers);
  if (callers.size() == 0) {
    return;
  }
  string name = InsnWriter::ResourceName(res_);
  WriteInputSel(name + "_s0", callers, 0, rs);
  WriteInputSel(name + "_s1", callers, 1, rs);
  WriteWire(name + "_d0", res_.output_types_[0], rs);

  rs << "  assign " << name << + "_d0 = "
     << name + "_s0 ";
  if (res_name == resource::kGt) {
    rs << ">";
  } else if (res_name == resource::kAdd) {
    rs << "+";
  } else if (res_name == resource::kSub) {
    rs << "-";
  } else if (res_name == resource::kMul) {
    rs << "*";
  } else if (res_name == resource::kEq) {
    rs << "==";
  } else if (res_name == resource::kGte) {
    rs << ">=";
  } else {
    LOG(FATAL) << "Unknown binop" << res_name;
  }
  rs << " " << name + "_s1;\n";
}

void Operator::BuildExclusiveBinOpInsn(IInsn *insn) {
  ostream &ws = tmpl_->GetStream(kInsnWireValueSection);
  ws << "  assign " << InsnWriter::InsnOutputWireName(*insn, 0)
     << " = " << InsnWriter::ResourceName(res_) << "_d0;\n";
}

void Operator::BuildLightUniOpInsn(IInsn *insn) {
  ostream &ws = tmpl_->GetStream(kInsnWireValueSection);
  ws << "  assign " << InsnWriter::InsnOutputWireName(*insn, 0)
     << " = ";
  const string &rc = insn->GetResource()->GetClass()->GetName();
  if (rc == resource::kBitInv) {
    ws << "~";
  } else {
    LOG(FATAL) << "Unknown LightUniOp: " << rc;
  }
  ws << InsnWriter::RegisterValue(*insn->inputs_[0], tab_.GetNames()) << ";\n";
}

void Operator::BuildLightBinOpInsn(IInsn *insn) {
  ostream &ws = tmpl_->GetStream(kInsnWireValueSection);
  ws << "  assign " << InsnWriter::InsnOutputWireName(*insn, 0)
     << " = " << InsnWriter::RegisterValue(*insn->inputs_[0], tab_.GetNames())
     << " ";
  const string &rc = insn->GetResource()->GetClass()->GetName();
  if (rc == resource::kBitAnd) {
    ws << "&";
  } else if (rc == resource::kBitOr) {
    ws << "|";
  } else if (rc == resource::kBitXor) {
    ws << "^";
  } else {
    LOG(FATAL) << "Unknown LightBinOp: " << rc;
  }
  ws << " " << InsnWriter::RegisterValue(*insn->inputs_[1], tab_.GetNames()) << ";\n";
}

void Operator::BuildBitShiftOpInsn(IInsn *insn) {
  ostream &ws = tmpl_->GetStream(kInsnWireValueSection);
  const string &rc = insn->GetResource()->GetClass()->GetName();
  if (rc != resource::kShift) {
    return;
  }
  bool is_left = (insn->GetOperand() == operand::kLeft);
  const IValue &value = insn->inputs_[1]->GetInitialValue();
  int amount = value.value_;
  ws << "  assign " << InsnWriter::InsnOutputWireName(*insn, 0)
     << " = "
     << InsnWriter::RegisterValue(*insn->inputs_[0], tab_.GetNames());
  if (is_left) {
    ws << " << ";
  } else {
    ws << " >> ";
  }
  ws << amount << ";\n";
}

void Operator::BuildBitSelInsn(IInsn *insn) {
  ostream &ws = tmpl_->GetStream(kInsnWireValueSection);
  string r = InsnWriter::RegisterValue(*insn->inputs_[0], tab_.GetNames());
  string msb = InsnWriter::ConstValue(*insn->inputs_[1]);
  string lsb = InsnWriter::ConstValue(*insn->inputs_[2]);
  ws << "  assign " << InsnWriter::InsnOutputWireName(*insn, 0)
     << " = " << r << "[" << msb << ":" << lsb << "];\n";
}

void Operator::BuildBitConcatInsn(IInsn *insn) {
  ostream &ws = tmpl_->GetStream(kInsnWireValueSection);
  vector<string> regs;
  for (IRegister *reg : insn->inputs_) {
    regs.push_back(InsnWriter::RegisterValue(*reg, tab_.GetNames()));
  }
  ws << "  assign " << InsnWriter::InsnOutputWireName(*insn, 0)
     << " = {" << Util::Join(regs, ", ") << "};\n";
}

void Operator::BuildSelectInsn(IInsn *insn) {
  ostream &ws = tmpl_->GetStream(kInsnWireValueSection);
  Names *names = tab_.GetNames();
  ws << "  assign " << InsnWriter::InsnOutputWireName(*insn, 0)
     << " = "
     << InsnWriter::RegisterValue(*insn->inputs_[0], names)
     << " ? "
     << InsnWriter::RegisterValue(*insn->inputs_[2], names)
     << " : "
     << InsnWriter::RegisterValue(*insn->inputs_[1], names)
     << ";\n";
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
